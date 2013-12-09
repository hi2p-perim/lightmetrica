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

#ifndef __LIB_NANON_VECTOR_H__
#define __LIB_NANON_VECTOR_H__

#include "common.h"

NANON_NAMESPACE_BEGIN

/*!
	4D vector.
	Generic 4-dimensional vector.
	\tparam T Internal value type
*/
template <typename T>
struct TVec4
{

	union
	{
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
		struct { T s, t, p, q; };
	};

	NANON_FORCE_INLINE TVec4();
	NANON_FORCE_INLINE TVec4(const TVec4<T>& v);
	NANON_FORCE_INLINE TVec4(const T& x, const T& y, const T& z, const T& w);

};

typedef TVec4<float> Vec4f;
typedef TVec4<double> Vec4d;
typedef TVec4<int> Vec4i;

NANON_NAMESPACE_END

#include "vector.inl"

#endif // __LIB_NANON_VECTOR_H__