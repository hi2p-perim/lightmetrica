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

#include "pch.h"
#include <lightmetrica/component.h>

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
		auto createFuncImplMap = createFuncMap.find(interfaceType);
		if (createFuncImplMap == createFuncMap.end())
		{
			return false;
		}

		return createFuncImplMap->second.find(implType) != createFuncImplMap->second.end();
	}

	bool CheckInterfaceRegistered(const std::string& interfaceType)
	{
		return createFuncMap.find(interfaceType) != createFuncMap.end();
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

private:

	typedef std::unordered_map<std::string, ComponentFactory::CreateComponentFunc> CreateComponentFuncImplMap;
	typedef std::unordered_map<std::string, CreateComponentFuncImplMap> CreateComponentFuncInterfaceMap;

	CreateComponentFuncInterfaceMap createFuncMap;

};

bool ComponentFactory::CheckRegistered( const std::string& interfaceType, const std::string& implType )
{
	return ComponentFactoryImpl::Instance().CheckRegistered(interfaceType, implType);
}

bool ComponentFactory::CheckInterfaceRegistered( const std::string& interfaceType )
{
	return ComponentFactoryImpl::Instance().CheckInterfaceRegistered(interfaceType);
}

bool ComponentFactory::Register( const std::string& interfaceType, const std::string& implType, const CreateComponentFunc& func )
{
	return ComponentFactoryImpl::Instance().Register(interfaceType, implType, func);
}

Component* ComponentFactory::Create( const std::string& interfaceType, const std::string& implType )
{
	return ComponentFactoryImpl::Instance().Create(interfaceType, implType);
}

LM_NAMESPACE_END