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

public:

	/*!
		Get component interface type.
		This function is automatically implemented with LM_COMPONENT_INTERFACE_DEF macro.
		\return Component interface type name.
		\sa LM_COMPONENT_INTERFACE_DEF.
	*/
	virtual std::string ComponentInterfaceTypeName() const = 0;

	/*!
		Get component implementation type.
		This function is automatically implemented with LM_COMPONENT_IMPL_DEF macro.
		\return Component implementation type name.
		\sa LM_COMPONENT_IMPL_DEF.
	*/
	virtual std::string ComponentImplTypeName() const = 0;

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
		\param interfaceType Component interface type.
		\param implType Component implementation type.
		\retval true Component of the given type is registered.
		\retval false Component of the given type is not registered.
	*/
	static bool CheckRegistered(const std::string& interfaceType, const std::string& implType);

	/*!
		Check if the component interface is registered.
		\param interfaceType Component interface type.
		\retval true Component interface of the given type is registered.
		\retval false Component interface of the given type is not registered.
	*/
	static bool CheckInterfaceRegistered(const std::string& interfaceType);

	/*!
		Register a component.
		Registers a component to the factory.
		Registered components can be instantiated with #Create function.
		\param interfaceType Component interface type.
		\param implType Component implementation type.
		\param func Instance creation function.
		\retval true Successfully registered.
		\retval false Failed to register a component.
	*/
	static bool Register(const std::string& interfaceType, const std::string& implType, const CreateComponentFunc& func);

	/*!
		Create an instance of a component.
		\param interfaceType Component interface type.
		\param implType Component implementation type.
		\return Instance. nullptr if not registered.
	*/
	static Component* Create(const std::string& interfaceType, const std::string& implType);

public:

	/*!
		Check if the component is registered.
		\tparam InterfaceType Interface type.
		\param interfaceType Component interface type.
		\param implType Component implementation type.
		\retval true Component of the given type is registered.
		\retval false Component of the given type is not registered.
	*/
	template <typename InterfaceType>
	static bool CheckRegistered(const std::string& implType)
	{
		std::string interfaceType = InterfaceType::InterfaceTypeName();
		return CheckRegistered(interfaceType, implType);
	}

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
		std::string interfaceType = InterfaceType::InterfaceTypeName();
		if (!CheckInterfaceRegistered(interfaceType))
		{
			LM_LOG_ERROR("Invalid interface type '" + interfaceType + "'");
			return nullptr;
		}

		auto* p1 = Create(interfaceType, implType);
		if (p1 == nullptr)
		{
			LM_LOG_ERROR("Invalid instance type '" + implType + "' (interface type : '" + interfaceType + "')");
			return nullptr;
		}

		auto* p2 = dynamic_cast<InterfaceType*>(p1);
		if (p2 == nullptr)
		{
			LM_LOG_ERROR("An instance of type '" + implType + "' is not inherited from '" + interfaceType + "'");
			LM_SAFE_DELETE(p1);
			return nullptr;
		}

		return p2;
	}

	/*!
		Create an instance of a component with interface type.
		This function creates default implementation for the givan interface.
		\tparam InterfaceType Interface type.
		\return Instance. nullptr if not registered, or failed to cast.
	*/
	template <typename InterfaceType>
	static InterfaceType* Create()
	{
		return Create<InterfaceType>("default");
	}

};

// --------------------------------------------------------------------------------

/*!
	Component factory entry.
	An entry for component factory.
	The singleton of the class is created for each pair of #ImplType and #InterfaceType.
	\tparam ImplType Component implementation type.
	\tparam InterfaceType Component interface type.
	\sa LM_COMPONENT_REGISTER_IMPL
*/
template <typename ImplType, typename InterfaceType>
class ComponentFactoryEntry
{
public:

	/*!
		Get instance.
		Obtains the singleton instance of the class.
	*/
	static ComponentFactoryEntry<ImplType, InterfaceType>& Instance()
	{
		static ComponentFactoryEntry<ImplType, InterfaceType> instance;
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
		if (!ComponentFactory::Register(InterfaceType::InterfaceTypeName(), ImplType::ImplTypeName(), [](){ return new ImplType; }))
		{
			LM_LOG_ERROR("Failed to register component '" + std::string(ImplType::ImplTypeName()) + "'");
		}
	}

private:

	ComponentFactoryEntry(const ComponentFactoryEntry<ImplType, InterfaceType>&);
	ComponentFactoryEntry(ComponentFactoryEntry<ImplType, InterfaceType>&&);
	void operator=(const ComponentFactoryEntry<ImplType, InterfaceType>&);
	void operator=(ComponentFactoryEntry<ImplType, InterfaceType>&&);

};

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

#define LM_COMPONENT_INTERFACE_DEF(Name)						\
	static const char* InterfaceTypeName() { return Name; }		\
	virtual std::string ComponentInterfaceTypeName() const { return InterfaceTypeName(); }

#define LM_COMPONENT_IMPL_DEF(Name)								\
	static const char* ImplTypeName() { return Name; }			\
	virtual std::string ComponentImplTypeName() const { return ImplTypeName(); }
	
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
	Statically checks whether #Type has a member function of #FuncName.
	Queries function must be registered beforehand.
	\param Type Type of the class to be checked.
	\param FuncName Function name.
	\sa LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION
*/

/*!
	\def LM_COMPONENT_CHECK_IS_DERIVED_CLASS(ImplType, InterfaceType)
	Statically checks whether #ImplType is inherited from #InterfaceType.
	\param ImplType Implementation class type.
	\param InterfaceType Interface class type.
*/

#define LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(FuncName, Signature)		\
	template <typename T, typename = std::true_type>						\
	struct has_member_function_##FuncName : std::false_type {};				\
																			\
	template <typename T>													\
	struct has_member_function_##FuncName<									\
		T,																	\
		std::integral_constant<												\
			bool,															\
			check_func_signature<Signature, &T::FuncName>::value			\
		>																	\
	> : std::true_type {}

#define LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(Type, FuncName)				\
	static_assert(															\
		has_member_function_##FuncName<Type>::value,						\
		"Component class of type '" #Type "' must have the member function '" #FuncName "'")

#define LM_COMPONENT_CHECK_IS_DERIVED_CLASS(ImplType, InterfaceType)		\
	static_assert(															\
		std::is_base_of<InterfaceType, ImplType>::value,					\
		"Component class of type '" #ImplType "' must be inherited from the class of type '" #InterfaceType "'")

// --------------------------------------------------------------------------------

LM_NAMESPACE_BEGIN

template <typename T, T>
struct check_func_signature : std::true_type {};

LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(ImplTypeName, const char* (*)());
LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(InterfaceTypeName, const char* (*)());

LM_NAMESPACE_END

// --------------------------------------------------------------------------------

/*!
	\def LM_COMPONENT_REGISTER_IMPL(ImplType, InterfaceType)
	Register implementation.
	Registered class must follow the following conditions:
	 - The class must define LM_COMPONENT_IMPL_DEF macro
	 - The class must be inherited from component interface class
	   in which defines LM_COMPONENT_INTERFACE_DEF macro
	\tparam ImplType Component implementation type.
	\tparam InterfaceType Component interface type.
*/

#define LM_COMPONENT_REGISTER_IMPL(ImplType, InterfaceType)										\
	namespace {																					\
																								\
		template <typename T1, typename T2>														\
		class ComponentFactoryEntryInstance;													\
																								\
		template <>																				\
		class ComponentFactoryEntryInstance<ImplType, InterfaceType>							\
		{																						\
			static const ::lightmetrica::ComponentFactoryEntry<ImplType, InterfaceType>& reg;	\
		};																						\
																								\
		const ::lightmetrica::ComponentFactoryEntry<ImplType, InterfaceType>&					\
			ComponentFactoryEntryInstance<ImplType, InterfaceType>::reg =						\
				::lightmetrica::ComponentFactoryEntry<ImplType, InterfaceType>::Instance();		\
																								\
		LM_COMPONENT_CHECK_IS_DERIVED_CLASS(ImplType, InterfaceType);							\
		LM_COMPONENT_CHECK_IS_DERIVED_CLASS(InterfaceType, Component);							\
		LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(ImplType, ImplTypeName);							\
		LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(InterfaceType, InterfaceTypeName);				\
																								\
	}

#endif // LIB_LIGHTMETRICA_COMPONENT_H