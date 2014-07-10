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
#ifndef LIB_LIGHTMETRICA_DYNAMIC_LIBRARY_H
#define LIB_LIGHTMETRICA_DYNAMIC_LIBRARY_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

/*!
	Dynamic library.
	Platform independent dynamic library class.
*/
class DynamicLibrary
{
public:
	
	DynamicLibrary();
	~DynamicLibrary();

private:

	LM_DISABLE_COPY_AND_MOVE(DynamicLibrary);

public:

	/*!
		Load a dynamic library.
		\param path Path to a library file.
		\retval true Succeeded to load.
		\retval false Failed to load.
	*/
	bool Load(const std::string& path);

	/*!
		Unload the dynamic library.
		\retval true Succeeded to unload.
		\retval false Failed to unload.
	*/
	bool Unload();
	
	/*!
		Get symbol address.
		Retrieve the address of an exported symbol.
		\retval nullptr Failed to get address.
	*/
	void* GetSymbolAddress(const std::string& symbol) const;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DYNAMIC_LIBRARY_H