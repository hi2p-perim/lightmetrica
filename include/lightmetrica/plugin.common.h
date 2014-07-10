/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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