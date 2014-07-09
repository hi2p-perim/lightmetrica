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
#include <lightmetrica/dynamiclibrary.h>
#include <lightmetrica/logger.h>
#if LM_PLATFORM_WINDOWS
#include <windows.h>
#endif

LM_NAMESPACE_BEGIN

class DynamicLibrary::Impl
{
public:

	Impl();

public:

	bool Load(const std::string& path);
	bool Unload();
	void* GetSymbolAddress(const std::string& symbol) const;

public:

	bool loaded;
	HMODULE handle;
	std::string path;

};

DynamicLibrary::Impl::Impl()
	: loaded(false)
{

}

bool DynamicLibrary::Impl::Load( const std::string& path )
{
	if (loaded)
	{
		LM_LOG_ERROR("Already loaded");
		return false;
	}

#if LM_PLATFORM_WINDOWS

	handle = LoadLibraryA(path.c_str());
	if (!handle)
	{
		LM_LOG_ERROR("LoadLibrary : " + path + " : " + std::to_string(GetLastError()));
		return false;
	}

	this->path = path;
	loaded = true;

	return true;

#else

	LM_LOG_WARN("TODO : Unsupported platform");
	return false;

#endif
}

bool DynamicLibrary::Impl::Unload()
{
	if (!loaded)
	{
		LM_LOG_ERROR("Not loaded");
		return false;
	}

#if LM_PLATFORM_WINDOWS

	if (!FreeLibrary(handle))
	{
		auto name = boost::filesystem::path(path).filename().string();
		LM_LOG_ERROR("FreeLibrary : " + name + " : " + std::to_string(GetLastError()));
		return false;
	}

	path = "";
	loaded = false;

	return true;

#else

	LM_LOG_WARN("TODO : Unsupported platform");
	return false;

#endif
}

void* DynamicLibrary::Impl::GetSymbolAddress( const std::string& symbol ) const
{
	if (!loaded)
	{
		LM_LOG_ERROR("Not loaded");
		return nullptr;
	}

#if LM_PLATFORM_WINDOWS

	void* address = GetProcAddress(handle, symbol.c_str());
	if (address == nullptr)
	{
		LM_LOG_ERROR("GetProcAddress : " + std::to_string(GetLastError()));
		return nullptr;
	}

	return address;

#else

	LM_LOG_WARN("TODO : Unsupported platform");
	return nullptr;

#endif
}

// --------------------------------------------------------------------------------

DynamicLibrary::DynamicLibrary()
	: p(new Impl)
{

}

DynamicLibrary::~DynamicLibrary()
{
	LM_SAFE_DELETE(p);
}

bool DynamicLibrary::Load( const std::string& path )
{
	return p->Load(path);
}

bool DynamicLibrary::Unload()
{
	return p->Unload();
}

void* DynamicLibrary::GetSymbolAddress( const std::string& symbol ) const
{
	return p->GetSymbolAddress(symbol);
}

LM_NAMESPACE_END