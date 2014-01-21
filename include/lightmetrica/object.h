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

#pragma once
#ifndef __LIB_LIGHTMETRICA_OBJECT_H__
#define __LIB_LIGHTMETRICA_OBJECT_H__

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

#endif // __LIB_LIGHTMETRICA_OBJECT_H__
