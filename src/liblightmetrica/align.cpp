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
#include "simdsupport.h"
#include <lightmetrica/align.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

void* aligned_malloc( size_t size, size_t align )
{
	void* p;
#ifdef LM_PLATFORM_WINDOWS
	p = _aligned_malloc(size, align);
#elif defined(LM_PLATFORM_LINUX)
    if ((align & (align - 1)) == 0 && align % sizeof(void*) == 0)
    {
        int result = posix_memalign(&p, align, size);
        if (result != 0)
        {
            p = nullptr;
#ifdef LM_DEBUG_MODE
            if (result == EINVAL)
            {
                LM_LOG_WARN("Alignment parameter is not a power of two multiple of sizeof(void*) : align = " + std::to_string(align));
            }
            else if (result == ENOMEM)
            {
                LM_LOG_DEBUG("Insufficient memory available");
            }
#endif
        }
    }
    else
    {
        // TODO
        p = malloc(size);
    }
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

void* SIMDAlignedType::operator new(std::size_t size) throw (std::bad_alloc) 
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

void SIMDAlignedType::operator delete(void* p)
{
#if (defined(LM_SINGLE_PRECISION) && defined(LM_USE_SSE2)) || (defined(LM_DOUBLE_PRECISION) && defined(LM_USE_AVX))
	aligned_free(p);
#else
	free(p);
#endif
}

LM_NAMESPACE_END

