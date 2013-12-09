/*
	nanon : A research-oriented renderer

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

#ifndef __LIB_NANON_SSE_VECTOR_H__
#define __LIB_NANON_SSE_VECTOR_H__

#include "vector.h"
#include "simdsupport.h"

#if !defined(NANON_USE_SSE) || !defined(NANON_USE_SSE2)
	#error "ssevector.h requires the support of SSE and SSE2"
#endif

#include <xmmintrin.h>

#ifdef NANON_USE_SSE3
#include <pmmintrin.h>
#endif
#ifdef NANON_USE_SSSE3
#include <tmmintrin.h>
#endif
#ifdef NANON_USE_SSE4_1
#include <smmintrin.h>
#endif
#ifdef NANON_USE_SSE4_2
#include <nmmintrin.h>
#endif
#ifdef NANON_USE_SSE4A
#include <ammintrin.h>
#endif

NANON_NAMESPACE_BEGIN

/*!
	SSE optimized 4D vector.
	Specialized version of TVec4 optimized by SSE.
*/
template <>
struct NANON_ALIGN_16 TVec4<float>
{

	union
	{
		__m128 v;
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		struct { float s, t, p, q; };
	};
	
	NANON_FORCE_INLINE TVec4();
	NANON_FORCE_INLINE TVec4(const Vec4f& v);
	NANON_FORCE_INLINE TVec4(__m128 v);
	NANON_FORCE_INLINE TVec4(float x, float y, float z, float w);

};

NANON_NAMESPACE_END

#include "ssevector.inl"

#endif // __LIB_NANON_SSE_VECTOR_H__