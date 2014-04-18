/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#pragma once
#ifndef LIB_LIGHTMETRICA_COMPONENT_H
#define LIB_LIGHTMETRICA_COMPONENT_H

#include "common.h"
#include "logger.h"
#include "align.h"
#include <string>
#include <functional>
#include <type_traits>

LM_NAMESPACE_BEGIN

/*!
	Component.
	The base class for component classes.
	Component system of lightmetrica renderer enable the implementer
	to reduce unnecessary headers and factory classes.
	The design of the system is inspired by the thread:
	http://gamedev.stackexchange.com/questions/17746/entity-component-systems-in-c-how-do-i-discover-types-and-construct-component
	
	Inherited classes are managed by intrusive reference counter
	in order to provide consistent memory management.
	All memory allocation is done in the shared object / dynamic library side.
*/
class Component : public SIMDAlignedType
{
public:

	Component() {}
	virtual ~Component() {}

};

// --------------------------------------------------------------------------------

/*!
	Component factory.
	Factory class for components.
	All components are managed by the factory.
*/
class LM_PUBLIC_API ComponentFactory
{
public:

	typedef std::function<Component* ()> CreateComponentFunc;

private:

	ComponentFactory() {}

private:

	LM_DISABLE_COPY_AND_MOVE(ComponentFactory);

public:
	
	/*!
		Check if the component is registered.
		\param implType Component implementation type.
		\retval true Component of the given type is registered.
		\retval false Component of the given type is not registered.
	*/
	static bool CheckRegistered(const std::string& implType);

	/*!
		Register a component.
		Registers a component to the factory.
		Registered components can be instantiated with #Create function.
		\param implType Component implementation type.
		\param func Instance creation function.
		\retval true Successfully registered.
		\retval false Failed to register a component.
	*/
	static bool Register(const std::string& implType, const CreateComponentFunc& func);

	/*!
		Create an instance of a component.
		\param implType Component implementation type.
		\return Instance. nullptr if not registered.
	*/
	static Component* Create(const std::string& implType);

public:

	/*!
		Create an instance of a component with interface type.
		Creates an instance of a component with the type of the given interface.
		\tparam InterfaceType Interface type.
		\param implType Component implementation type.
		\return Instance. nullptr if not registered, or failed to cast.
	*/
	template <typename InterfaceType>
	static InterfaceType* Create(const std::string& implType)
	{
		auto* p1 = Create(implType);
		if (p1 == nullptr)
		{
			LM_LOG_ERROR("Invalid instance type '" + implType + "'");
			return nullptr;
		}

		auto* p2 = dynamic_cast<InterfaceType*>(p1);
		if (p2 == nullptr)
		{
			LM_LOG_ERROR("An instance of type '" + implType +
				"' is not inherited from '" + InterfaceType::InterfaceTypeName() + "'");
			LM_SAFE_DELETE(p1);
			return nullptr;
		}

		return p2;
	}

};

// --------------------------------------------------------------------------------

LM_DETAIL_NAMESPACE_BEGIN

/*!
	Component factory entry.
	An entry for component factory.
	The singleton of the class is created per #ImplType
	from the macro for registration.
	\tparam ImplType Component implementation type.
	\sa LM_COMPONENT_REGISTER_IMPL
*/
template <typename ImplType>
class ComponentFactoryEntry
{
public:

	/*!
		Get instance.
		Obtains the singleton instance of the class.
	*/
	static ComponentFactoryEntry<ImplType>& Instance()
	{
		static ComponentFactoryEntry<ImplType> instance;
		return instance;
	}

private:

	/*!
		Constructor.
		The implementation is registered to the component factory when constructed.
	*/
	ComponentFactoryEntry()
	{
		// Register entry here
		if (!ComponentFactory::Register(ImplType::ImplTypeName(), [](){ return new ImplType; }))
		{
			LM_LOG_ERROR("Failed to register component '" + std::string(ImplType::ImplTypeName()) + "'");
		}
	}

private:

	ComponentFactoryEntry(const ComponentFactoryEntry<ImplType>&);
	ComponentFactoryEntry(ComponentFactoryEntry<ImplType>&&);
	void operator=(const ComponentFactoryEntry<ImplType>&);
	void operator=(ComponentFactoryEntry<ImplType>&&);

};

LM_DETAIL_NAMESPACE_END
LM_NAMESPACE_END

// --------------------------------------------------------------------------------

/*!
	\def LM_COMPONENT_INTERFACE_DEF(Name)
	Defines component interface.
	The macro must be placed in the interface class with public access.
	\param Name Name of the class.
*/

/*!
	\def LM_COMPONENT_IMPL_DEF(Name)
	Defines component implementation.
	The macro must be place in the implementation class with public access.
	\param Name of the class.
*/

#define LM_COMPONENT_INTERFACE_DEF(Name)												\
	static const char* InterfaceTypeName() { return Name; }

#define LM_COMPONENT_IMPL_DEF(Name)														\
	static const char* ImplTypeName() { return Name; }
	
// --------------------------------------------------------------------------------

/*!
	\def LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(FuncName, Signature)
	Defines a function that checks whether a class has a member function.
	\param FuncName Function name.
	\param Signature Type of the function.
	\sa LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION
*/

/*!
	\def LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(Type, FuncName)
	Checks whether a class with #Type has a member function of #FuncName.
	Queries function must be registered beforehand.
	\param Type Type of the class to be checked.
	\param FuncName Function name.
	\sa LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION
*/

#define LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(FuncName, Signature)					\
	template <typename T, typename = std::true_type>									\
	struct has_member_function_##FuncName : std::false_type {};							\
																						\
	template <typename T>																\
	struct has_member_function_##FuncName<												\
		T,																				\
		std::integral_constant<															\
			bool,																		\
			check_func_signature<Signature, &T::FuncName>::value						\
		>																				\
	> : std::true_type {}

#define LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(Type, FuncName)							\
	static_assert(																		\
		has_member_function_##FuncName<Type>::value,									\
		"Component class of type '" #Type "' must have the member function '" #FuncName "'")

// --------------------------------------------------------------------------------

LM_NAMESPACE_BEGIN
LM_DETAIL_NAMESPACE_BEGIN

template <typename T, T>
struct check_func_signature : std::true_type {};

LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(ImplTypeName, const char* (*)());

LM_DETAIL_NAMESPACE_END
LM_NAMESPACE_END

// --------------------------------------------------------------------------------

/*!
	\def LM_COMPONENT_REGISTER_IMPL(ImplType)
	Register implementation.
	Registered class must follow the following conditions:
	 - The class must define LM_COMPONENT_IMPL_DEF macro
	 - The class must be inherited from component interface class
	   in which defines LM_COMPONENT_INTERFACE_DEF macro
	\tparam ImplType Component implementation type.
*/

#define LM_COMPONENT_REGISTER_IMPL(ImplType)											\
	namespace detail {																	\
	namespace {																			\
																						\
		template <typename T>															\
		class ComponentFactoryEntryInstance;											\
																						\
		template <>																		\
		class ComponentFactoryEntryInstance<ImplType>									\
		{																				\
			static const ::lightmetrica::detail::ComponentFactoryEntry<ImplType>& reg;	\
		};																				\
																						\
		const ::lightmetrica::detail::ComponentFactoryEntry<ImplType>&					\
			ComponentFactoryEntryInstance<ImplType>::reg =								\
				::lightmetrica::detail::ComponentFactoryEntry<ImplType>::Instance();	\
																						\
		LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(ImplType, ImplTypeName);					\
																						\
	}}

#endif // LIB_LIGHTMETRICA_COMPONENT_H