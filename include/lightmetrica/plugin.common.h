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
#ifndef LIB_LIGHTMETRICA_PLUGIN_COMMON_H
#define LIB_LIGHTMETRICA_PLUGIN_COMMON_H

#include "component.h"
#include <string>
#include <functional>

LM_NAMESPACE_BEGIN

class PluginManager
{
public:

	typedef std::function<Component* ()> CreateComponentFunc;

private:

	PluginManager() {}

private:

	LM_DISABLE_COPY_AND_MOVE(PluginManager);

public:

	static bool Register(const std::string& interfaceType, const std::string& implType, const CreateComponentFunc& func);

};

template <typename ImplType, typename InterfaceType>
class ComponentPluginFactoryEntry
{
public:

	static ComponentPluginFactoryEntry<ImplType, InterfaceType>& Instance()
	{
		static ComponentPluginFactoryEntry<ImplType, InterfaceType> instance;
		return instance;
	}

private:

	ComponentPluginFactoryEntry()
	{
		if (!PluginManager::Register(InterfaceType::InterfaceTypeName(), ImplType::ImplTypeName(), [](){ return new ImplType; }))
		{
			LM_LOG_ERROR("Failed to register plugin '" + std::string(ImplType::ImplTypeName()) + "'");
		}
	}

private:

	ComponentPluginFactoryEntry(const ComponentPluginFactoryEntry<ImplType, InterfaceType>&);
	ComponentPluginFactoryEntry(ComponentPluginFactoryEntry<ImplType, InterfaceType>&&);
	void operator=(const ComponentPluginFactoryEntry<ImplType, InterfaceType>&);
	void operator=(ComponentPluginFactoryEntry<ImplType, InterfaceType>&&);

};

LM_NAMESPACE_END

#define LM_COMPONENT_REGISTER_PLUGIN_IMPL(ImplType, InterfaceType)									\
	namespace {																						\
																									\
		template <typename T1, typename T2>															\
		class ComponentPluginFactoryEntryInstance;													\
																									\
		template <>																					\
		class ComponentPluginFactoryEntryInstance<ImplType, InterfaceType>							\
		{																							\
			static const ::lightmetrica::ComponentPluginFactoryEntry<ImplType, InterfaceType>& reg;	\
		};																							\
																									\
		const ::lightmetrica::ComponentPluginFactoryEntry<ImplType, InterfaceType>&					\
			ComponentPluginFactoryEntryInstance<ImplType, InterfaceType>::reg =						\
				::lightmetrica::ComponentPluginFactoryEntry<ImplType, InterfaceType>::Instance();	\
																									\
		LM_COMPONENT_CHECK_IS_DERIVED_CLASS(ImplType, InterfaceType);								\
		LM_COMPONENT_CHECK_IS_DERIVED_CLASS(InterfaceType, Component);								\
		LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(ImplType, ImplTypeName);								\
		LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(InterfaceType, InterfaceTypeName);					\
																									\
	}

#endif // LIB_LIGHTMETRICA_PLUGIN_COMMON_H