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

#include "math.common.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

/*!
	2D vector.
	Generic 2-dimensional vector.
	\tparam T Internal value type.
*/
template <typename T>
struct TVec2
{

	T x, y;

	NANON_FORCE_INLINE TVec2();
	NANON_FORCE_INLINE TVec2(const TVec2<T>& v);
	NANON_FORCE_INLINE TVec2(const T& v);
	NANON_FORCE_INLINE TVec2(const T& x, const T& y);
	NANON_FORCE_INLINE T& operator[](int i);
	NANON_FORCE_INLINE const T& operator[](int i) const;
	NANON_FORCE_INLINE TVec2<T>& operator=(const TVec2<T>& v);

};

typedef TVec2<float> Vec2f;
typedef TVec2<double> Vec2d;
typedef TVec2<int> Vec2i;

// --------------------------------------------------------------------------------

/*!
	3D vector.
	Generic 3-dimensional vector.
	\tparam T Internal value type.
*/
template <typename T>
struct TVec3
{

	T x, y, z;

	NANON_FORCE_INLINE TVec3();
	NANON_FORCE_INLINE TVec3(const TVec3<T>& v);
	NANON_FORCE_INLINE TVec3(const T& v);
	NANON_FORCE_INLINE TVec3(const T& x, const T& y, const T& z);
	NANON_FORCE_INLINE T& operator[](int i);
	NANON_FORCE_INLINE const T& operator[](int i) const;
	NANON_FORCE_INLINE TVec3<T>& operator=(const TVec3<T>& v);

};

typedef TVec3<float> Vec3f;
typedef TVec3<double> Vec3d;
typedef TVec3<int> Vec3i;

// --------------------------------------------------------------------------------

/*!
	4D vector.
	Generic 4-dimensional vector.
	\tparam T Internal value type
*/
template <typename T>
struct TVec4
{

	T x, y, z, w;

	NANON_FORCE_INLINE TVec4();
	NANON_FORCE_INLINE TVec4(const TVec4<T>& v);
	NANON_FORCE_INLINE TVec4(const T& v);
	NANON_FORCE_INLINE TVec4(const T& x, const T& y, const T& z, const T& w);
	NANON_FORCE_INLINE T& operator[](int i);
	NANON_FORCE_INLINE const T& operator[](int i) const;
	NANON_FORCE_INLINE TVec4<T>& operator=(const TVec4<T>& v);

};

typedef TVec4<float> Vec4f;
typedef TVec4<double> Vec4d;
typedef TVec4<int> Vec4i;

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#include "math.vector.inl"

#endif // __LIB_NANON_VECTOR_H__