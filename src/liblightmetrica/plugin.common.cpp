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

#include "pch.plugin.h"
#include <lightmetrica/plugin.common.h>
#include <unordered_map>

LM_NAMESPACE_BEGIN

class PluginManagerImpl
{
public:

	static PluginManagerImpl& Instance()
	{
		static PluginManagerImpl instance;
		return instance;
	}

	bool Register(const std::string& interfaceType, const std::string& implType, const PluginManager::CreateComponentFunc& func)
	{
		auto createFuncImplMapIter = createFuncMap.find(interfaceType);
		if (createFuncImplMapIter != createFuncMap.end())
		{
			auto createFuncIter = createFuncImplMapIter->second.find(implType);
			if (createFuncIter != createFuncImplMapIter->second.end())
			{
				return false;
			}
		}

		createFuncMap[interfaceType][implType] = func;

		return true;
	}

	Component* CreateInstance(const std::string& implType, const std::string& interfaceType)
	{
		auto createFuncImplMapIter = createFuncMap.find(interfaceType);
		if (createFuncImplMapIter == createFuncMap.end())
		{
			return nullptr;
		}

		auto createFuncIter = createFuncImplMapIter->second.find(implType);
		if (createFuncIter == createFuncImplMapIter->second.end())
		{
			return nullptr;
		}

		return createFuncIter->second();
	}

private:

	typedef std::unordered_map<std::string, ComponentFactory::CreateComponentFunc> CreateComponentFuncImplMap;
	typedef std::unordered_map<std::string, CreateComponentFuncImplMap> CreateComponentFuncInterfaceMap;

	CreateComponentFuncInterfaceMap createFuncMap;				// For internal classes

};

bool PluginManager::Register( const std::string& interfaceType, const std::string& implType, const CreateComponentFunc& func )
{
	return PluginManagerImpl::Instance().Register(interfaceType, implType, func);
}

LM_NAMESPACE_END

extern "C"
{

	LM_PLUGIN_API lightmetrica::Component* LM_Plugin_CreateInstance(const char* implType, const char* interfaceType)
	{
		return lightmetrica::PluginManagerImpl::Instance().CreateInstance(implType, interfaceType);
	}

}