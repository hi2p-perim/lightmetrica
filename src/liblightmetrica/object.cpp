/*
	L I G H T  M E T R I C A

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
#include "simdsupport.h"
#include <lightmetrica/object.h>
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

void* Object::operator new(std::size_t size) throw (std::bad_alloc) 
{
#if defined(LM_SINGLE_PRECISION) && defined(LM_USE_SSE2)
	void* p = aligned_malloc(size, 16);
#elif defined(LM_DOUBLE_PRECISION) && defined(LM_USE_AVX)
	void* p = aligned_malloc(size, 32);
#else
	void* p = malloc(size);
#endif
	if (!p) throw std::bad_alloc();
	return p;
}

void Object::operator delete(void* p)
{
#if (defined(LM_SINGLE_PRECISION) && defined(LM_USE_SSE2)) || (defined(LM_DOUBLE_PRECISION) && defined(LM_USE_AVX))
	aligned_free(p);
#else
	free(p);
#endif
}

LM_NAMESPACE_END
