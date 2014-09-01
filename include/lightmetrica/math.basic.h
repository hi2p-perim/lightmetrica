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
#ifndef LIB_LIGHTMETRICA_MATH_BASIC_H
#define LIB_LIGHTMETRICA_MATH_BASIC_H

#include "math.common.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> LM_FORCE_INLINE T Radians(const T& v);
template <typename T> LM_FORCE_INLINE T Degrees(const T& v);
template <typename T> LM_FORCE_INLINE T Cos(const T& v);
template <typename T> LM_FORCE_INLINE T Sin(const T& v);
template <typename T> LM_FORCE_INLINE T Tan(const T& v);
template <typename T> LM_FORCE_INLINE T Abs(const T& v);
template <typename T> LM_FORCE_INLINE T Sqrt(const T& v);
template <typename T> LM_FORCE_INLINE T Log(const T& v);
template <typename T> LM_FORCE_INLINE T Exp(const T& v);
template <typename T> LM_FORCE_INLINE T Pow(const T& base, const T& exp);
template <typename T> LM_FORCE_INLINE T Min(const T& v1, const T& v2);
template <typename T> LM_FORCE_INLINE T Max(const T& v1, const T& v2);
template <typename T> LM_FORCE_INLINE T Clamp(const T& v, const T& min, const T& max);
template <typename T> LM_FORCE_INLINE T Fract(const T& v);
template <typename T> LM_FORCE_INLINE bool IsZero(const T& v);

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.basic.inl"

#endif // LIB_LIGHTMETRICA_MATH_BASIC_H