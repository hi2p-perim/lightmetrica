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
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

void* aligned_malloc( size_t size, size_t align )
{
	void* p;
#ifdef LM_PLATFORM_WINDOWS
	p = _aligned_malloc(size, align);
#elif defined(LM_PLATFORM_LINUX)
	if (posix_memalign(&p, align, size)) p = nullptr;
#else
	// cf.
	// http://www.songho.ca/misc/alignment/dataalign.html
	// http://stackoverflow.com/questions/227897/solve-the-memory-alignment-in-c-interview-question-that-stumped-me
	// http://cottonvibes.blogspot.jp/2011/01/dynamically-allocate-aligned-memory.html
	uintptr_t r = (uintptr_t)malloc(size + --align + sizeof(uintptr_t));
	uintptr_t t = r + sizeof(uintptr_t);
	uintptr_t o = (t + align) & ~(uintptr_t)align;
	if (!r) return nullptr;
	((uintptr_t*)o)[-1] = r;
	p = (void*)o;
#endif
	return p;
}

void aligned_free( void* p )
{
#ifdef LM_PLATFORM_WINDOWS
	_aligned_free(p);
#elif defined(LM_PLATFORM_LINUX)
	free(p);
#else
	if (!p) return;
	free((void*)(((uintptr_t*)p)[-1]));
#endif
}

LM_NAMESPACE_END

