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