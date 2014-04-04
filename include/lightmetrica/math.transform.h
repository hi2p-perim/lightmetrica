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
#ifndef LIB_LIGHTMETRICA_MATH_TRANSFORM_H
#define LIB_LIGHTMETRICA_MATH_TRANSFORM_H

#include "math.matrix.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> LM_FORCE_INLINE TMat4<T> Translate(const TMat4<T>& m, const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TMat4<T> Translate(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TMat4<T> Rotate(const TMat4<T>& m, T angle, const TVec3<T>& axis);
template <typename T> LM_FORCE_INLINE TMat4<T> Rotate(T angle, const TVec3<T>& axis);
template <typename T> LM_FORCE_INLINE TMat4<T> Scale(const TMat4<T>& m, const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TMat4<T> Scale(const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TMat4<T> LookAt(const TVec3<T>& eye, const TVec3<T>& center, const TVec3<T>& up);
template <typename T> LM_FORCE_INLINE TMat4<T> Perspective(T fovy, T aspect, T zNear, T zFar);

#ifdef LM_USE_SSE2
template <> LM_FORCE_INLINE Mat4f Rotate(const Mat4f& m, float angle, const Vec3f& axis);
template <> LM_FORCE_INLINE Mat4f Rotate(float angle, const Vec3f& axis);
#endif

#ifdef LM_USE_AVX
template <> LM_FORCE_INLINE Mat4d Rotate(const Mat4d& m, double angle, const Vec3d& axis);
template <> LM_FORCE_INLINE Mat4d Rotate(double angle, const Vec3d& axis);
#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.transform.inl"

#endif // LIB_LIGHTMETRICA_MATH_TRANSFORM_H