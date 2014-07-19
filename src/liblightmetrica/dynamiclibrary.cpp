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
#include <lightmetrica/dynamiclibrary.h>
#include <lightmetrica/logger.h>
#if LM_PLATFORM_WINDOWS
#include <windows.h>
#elif LM_PLATFORM_LINUX
#include <dlfcn.h>
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
#if LM_PLATFORM_WINDOWS
	HMODULE handle;
#elif LM_PLATFORM_LINUX
    void* handle;
#endif
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
		auto filename = boost::filesystem::path(path).filename().string();
		LM_LOG_ERROR("Failed to load library " + filename + " : " + std::to_string(GetLastError()));
		return false;
	}
#elif LM_PLATFORM_LINUX
	handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
	if (!handle)
	{
		auto filename = boost::filesystem::path(path).filename().string();
		LM_LOG_ERROR("Failed to load library '" + filename + "' : " + std::string(dlerror()));
		return false;
	}
#endif

	this->path = path;
	loaded = true;

	return true;
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
		auto filename = boost::filesystem::path(path).filename().string();
		LM_LOG_ERROR("Failed to free library : '" + filename + "' : " + std::to_string(GetLastError()));
		return false;
	}
#elif LM_PLATFORM_LINUX
	if (dlclose(handle) != 0)
	{
		auto filename = boost::filesystem::path(path).filename().string();
		LM_LOG_ERROR("Failed to free library : '" + filename + "' : " + std::string(dlerror()));
		return false;
	}
#endif

	path = "";
	loaded = false;

	return true;
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
		LM_LOG_ERROR("Failed to get address of '" + symbol + "' : " + std::to_string(GetLastError()));
		return nullptr;
	}
#elif LM_PLATFORM_LINUX
	void* address = dlsym(handle, symbol.c_str());
	if (address == nullptr)
	{
		LM_LOG_ERROR("Failed to get address of '" + symbol + "' : " + std::string(dlerror()));
		return nullptr;
	}
#endif

	return address;
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
