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

#if LM_SSE2
template <> LM_FORCE_INLINE Mat4f Rotate(const Mat4f& m, float angle, const Vec3f& axis);
template <> LM_FORCE_INLINE Mat4f Rotate(float angle, const Vec3f& axis);
#endif

#if LM_AVX
template <> LM_FORCE_INLINE Mat4d Rotate(const Mat4d& m, double angle, const Vec3d& axis);
template <> LM_FORCE_INLINE Mat4d Rotate(double angle, const Vec3d& axis);
#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.transform.inl"

#endif // LIB_LIGHTMETRICA_MATH_TRANSFORM_H