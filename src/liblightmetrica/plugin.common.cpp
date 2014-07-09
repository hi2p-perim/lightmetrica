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