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
#include <lightmetrica/randomfactory.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/standardmtrand.h>
#include <lightmetrica/sfmtrand.h>

LM_NAMESPACE_BEGIN

class RandomFactoryImpl
{
public:

	static RandomFactoryImpl& Instance()
	{
		static RandomFactoryImpl instance;
		return instance;
	}

public:

	RandomFactoryImpl()
	{
		AddFactory<StandardMTRandom>();
		AddFactory<SFMTRandom>();
	}

public:

	Random* Create(const std::string& type)
	{
		if (!CheckSupport(type))
		{
			LM_LOG_ERROR("Invalid random number type '" + type + "'");
			return nullptr;
		}
		else
		{
			return factoryMap[type]();
		}
	}

	bool CheckSupport(const std::string& type)
	{
		return factoryMap.find(type) != factoryMap.end();
	}

private:

	template <typename RandomImpl>
	void AddFactory()
	{
		factoryMap[RandomImpl::StaticType()] = [](){ return new RandomImpl(); };
	}

private:

	std::unordered_map<std::string, std::function<Random* ()>> factoryMap;

};

// --------------------------------------------------------------------------------

Random* RandomFactory::Create( const std::string& type )
{
	return RandomFactoryImpl::Instance().Create(type);
}

bool RandomFactory::CheckSupport( const std::string& type )
{
	return RandomFactoryImpl::Instance().CheckSupport(type);
}

LM_NAMESPACE_END