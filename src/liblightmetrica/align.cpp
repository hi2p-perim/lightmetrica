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
#include "simdsupport.h"
#include <lightmetrica/align.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

void* aligned_malloc( size_t size, size_t align )
{
	void* p;
#if LM_PLATFORM_WINDOWS
	p = _aligned_malloc(size, align);
#elif LM_PLATFORM_LINUX
    if ((align & (align - 1)) == 0 && align % sizeof(void*) == 0)
    {
        int result = posix_memalign(&p, align, size);
        if (result != 0)
        {
            p = nullptr;
#if LM_DEBUG_MODE
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
#if LM_PLATFORM_WINDOWS
	_aligned_free(p);
#elif LM_PLATFORM_LINUX
	free(p);
#else
	if (!p) return;
	free((void*)(((uintptr_t*)p)[-1]));
#endif
}

void* SIMDAlignedType::operator new(std::size_t size) throw (std::bad_alloc) 
{
#if LM_SINGLE_PRECISION && LM_SSE2
	void* p = aligned_malloc(size, 16);
#elif LM_DOUBLE_PRECISION && LM_AVX
	void* p = aligned_malloc(size, 32);
#else
	void* p = malloc(size);
#endif
	if (!p) throw std::bad_alloc();
	return p;
}

void SIMDAlignedType::operator delete(void* p)
{
#if (LM_SINGLE_PRECISION && LM_SSE2) || (LM_DOUBLE_PRECISION && LM_AVX)
	aligned_free(p);
#else
	free(p);
#endif
}

LM_NAMESPACE_END

