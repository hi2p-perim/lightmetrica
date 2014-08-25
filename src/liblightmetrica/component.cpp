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

#include "pch.h"
#include <lightmetrica/component.h>
#include <lightmetrica/dynamiclibrary.h>
#include <boost/regex.hpp>

LM_NAMESPACE_BEGIN

class ComponentFactoryImpl
{
public:

	static ComponentFactoryImpl& Instance()
	{
		static ComponentFactoryImpl instance;
		return instance;
	}

public:

	bool CheckRegistered(const std::string& interfaceType, const std::string& implType)
	{
		return CheckRegisteredFromInternal(interfaceType, implType) || CheckRegisteredFromPlugin(interfaceType, implType);
	}

	bool Register(const std::string& interfaceType, const std::string& implType, const ComponentFactory::CreateComponentFunc& func)
	{
		if (CheckRegistered(interfaceType, implType))
		{
			return false;
		}

		createFuncMap[interfaceType][implType] = func;
		return true;
	}

	Component* Create(const std::string& interfaceType, const std::string& implType)
	{
		auto* instance = CreateInstanceFromInternal(interfaceType, implType);
		return instance ? instance : CreateInstanceFromPlugin(interfaceType, implType);
	}

	void LoadPlugins(const std::string& pluginDir)
	{
		namespace fs = boost::filesystem;

		// File format : 'plugin.(name).{dll,so}'
#if LM_PLATFORM_WINDOWS
		const boost::regex pluginNameExp("^plugin\\.([a-z]+)\\.dll$");
#elif LM_PLATFORM_LINUX
		const boost::regex pluginNameExp("^plugin\\.([a-z]+)\\.so$");
#endif

		// Enumerate dynamic libraries in #pluginDir
		fs::directory_iterator endIter;
		for (fs::directory_iterator it(pluginDir); it != endIter; ++it)
		{
			if (fs::is_regular_file(it->status()))
			{
				boost::cmatch match;
				auto filename = it->path().filename().string();
				if (boost::regex_match(filename.c_str(), match, pluginNameExp))
				{
					LM_LOG_INFO("Loading '" + filename + "'");
					LM_LOG_INDENTER();

					// Load plugin
					std::unique_ptr<DynamicLibrary> library(new DynamicLibrary);
					if (!library->Load(it->path().string()))
					{
						LM_LOG_WARN("Failed to load library, skipping.");
						continue;
					}

					// Load symbol 'LM_Plugin_CreateInstance'
					void* factoryFuncSym = library->GetSymbolAddress("LM_Plugin_CreateInstance");
					if (factoryFuncSym == nullptr)
					{
						LM_LOG_ERROR("Failed to find symbol 'LM_Plugin_CreateInstance', skipping.");
						continue;
					}

					// Load symbol 'LM_Plugin_CreateInstance'
					void* checkFuncSym = library->GetSymbolAddress("LM_Plugin_CheckRegistered");
					if (checkFuncSym == nullptr)
					{
						LM_LOG_ERROR("Failed to find symbol 'LM_Plugin_CheckRegistered', skipping.");
						continue;
					}

					libraries.push_back(std::move(library));

					typedef Component* (*PluginCreateInstanceFunction)(const char*, const char*);
					typedef bool (*PluginCheckRegisteredFunction)(const char*, const char*);
					pluginCreateInstanceFuncs.emplace_back(reinterpret_cast<PluginCreateInstanceFunction>(factoryFuncSym));
					pluginCheckRegisteredFuncs.emplace_back(reinterpret_cast<PluginCheckRegisteredFunction>(checkFuncSym));

					LM_LOG_INFO("Successfully loaded");
				}
			}
		}
	}

	void UnloadPlugins()
	{
		for (auto& library : libraries)
		{
			library->Unload();
		}

		libraries.clear();
		pluginCreateInstanceFuncs.clear();
	}

private:

	Component* CreateInstanceFromInternal(const std::string& interfaceType, const std::string& implType)
	{
		auto createFuncImplMap = createFuncMap.find(interfaceType);
		if (createFuncImplMap == createFuncMap.end())
		{
			return nullptr;
		}

		auto createFunc = createFuncImplMap->second.find(implType);
		if (createFunc == createFuncImplMap->second.end())
		{
			return nullptr;
		}

		return createFunc->second();
	}

	Component* CreateInstanceFromPlugin(const std::string& interfaceType, const std::string& implType)
	{
#if 1
		// TODO : Try other ways if it degrades performance
		for (auto& createFunc : pluginCreateInstanceFuncs)
		{
			auto instance = createFunc(implType.c_str(), interfaceType.c_str());
			if (instance != nullptr)
			{
				return instance;
			}
		}

		return nullptr;
#else
		// Check if required symbol is already loaded
		bool requireLoad = false;
		ComponentFactory::CreateComponentFunc createFunc;
		
		auto createFuncImplMapIter = pluginCreateFuncMap.find(interfaceType);
		if (createFuncImplMapIter == pluginCreateFuncMap.end())
		{
			requireLoad = true;
		}

		if (!requireLoad)
		{
			auto createFuncIter = createFuncImplMapIter->second.find(implType);
			if (createFuncIter == createFuncImplMapIter->second.end())
			{
				requireLoad = true;
			}
			else
			{
				createFunc = createFuncIter->second;
			}
		}

		// Load symbol if required
		if (requireLoad)
		{
			// Retrieve factory function
			// Format : 'LM_CreateInstance_(implementation)_(interface)'
			bool found = false;
			auto factoryFuncSymName = "LM_CreateInstance_" + implType + "_" + interfaceType;
			for (auto& library : libraries)
			{
				void* factoryFuncSym = library->GetSymbolAddress(factoryFuncSymName);
				if (factoryFuncSym == nullptr)
				{
					continue;
				}

				found = true;
				typedef Component* (*CreateInstanceFunction)();
				createFunc = static_cast<CreateInstanceFunction>(factoryFuncSym);
				createFuncMap[interfaceType][implType] = createFunc;

				break;
			}

			if (!found)
			{
				LM_LOG_ERROR("Failed to find symbol '" + factoryFuncSymName + "'");
				return nullptr;
			}
		}

		return createFunc();
#endif
	}

	bool CheckRegisteredFromInternal(const std::string& interfaceType, const std::string& implType)
	{
		auto createFuncImplMap = createFuncMap.find(interfaceType);
		if (createFuncImplMap == createFuncMap.end())
		{
			return false;
		}

		return createFuncImplMap->second.find(implType) != createFuncImplMap->second.end();
	}

	bool CheckRegisteredFromPlugin(const std::string& interfaceType, const std::string& implType)
	{
		for (auto& checkRegisteredFunc : pluginCheckRegisteredFuncs)
		{
			bool ret = checkRegisteredFunc(implType.c_str(), interfaceType.c_str());
			if (ret)
			{
				return true;
			}
		}

		return false;
	}

private:

	typedef std::unordered_map<std::string, ComponentFactory::CreateComponentFunc> CreateComponentFuncImplMap;
	typedef std::unordered_map<std::string, CreateComponentFuncImplMap> CreateComponentFuncInterfaceMap;
	
	CreateComponentFuncInterfaceMap createFuncMap;									// For internal classes
	//CreateComponentFuncInterfaceMap pluginCreateFuncMap;							// For plugins
	std::vector<std::unique_ptr<DynamicLibrary>> libraries;							// Loaded dynamic libraries
	
	typedef std::function<Component* (const char*, const char*)> PluginCreateComponentFunc;
	typedef std::function<bool (const char*, const char*)> PluginCheckRegisteredFunc;
	std::vector<PluginCreateComponentFunc> pluginCreateInstanceFuncs;
	std::vector<PluginCheckRegisteredFunc> pluginCheckRegisteredFuncs;

};

bool ComponentFactory::CheckRegistered( const std::string& interfaceType, const std::string& implType )
{
	return ComponentFactoryImpl::Instance().CheckRegistered(interfaceType, implType);
}

bool ComponentFactory::Register( const std::string& interfaceType, const std::string& implType, const CreateComponentFunc& func )
{
	return ComponentFactoryImpl::Instance().Register(interfaceType, implType, func);
}

Component* ComponentFactory::Create( const std::string& interfaceType, const std::string& implType )
{
	return ComponentFactoryImpl::Instance().Create(interfaceType, implType);
}

void ComponentFactory::LoadPlugins( const std::string& pluginDir )
{
	return ComponentFactoryImpl::Instance().LoadPlugins(pluginDir);
}

void ComponentFactory::UnloadPlugins()
{
	return ComponentFactoryImpl::Instance().UnloadPlugins();
}

LM_NAMESPACE_END