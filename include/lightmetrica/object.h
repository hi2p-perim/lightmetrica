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
#ifndef LIB_LIGHTMETRICA_OBJECT_H
#define LIB_LIGHTMETRICA_OBJECT_H

#include "common.h"
#include <cstddef>
#include <exception>
#include <new>

LM_NAMESPACE_BEGIN

/*!
	Object.
	The base class of the all classes.
	The class offers:
	- Reference counting
	- Operator overriding for new and delete
	  - Aligned allocation for SIMD data types
	  - Enables to call new and delete from the outside library across the DLL boundary
*/
class LM_PUBLIC_API Object
{
public:

	virtual ~Object() {}

public:

    // TODO : Using global aligned allocator increases performance penalty
    // and it is not a clean design ... Re-design them somehow.
	void* operator new(std::size_t size) throw (std::bad_alloc);
	void operator delete(void* p);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_OBJECT_H
