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
#ifndef LIB_LIGHTMETRICA_MATH_COLORS_H
#define LIB_LIGHTMETRICA_MATH_COLORS_H

#include "math.vector.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

/*!
	Useful color constants for TVec3.
	For internal use.
	\tparam T Internal value type.
*/
template <typename T>
class TColors
{
public:

	TColors();
	~TColors();

	LM_DISABLE_COPY_AND_MOVE(TColors);

public:

	LM_FORCE_INLINE static TVec3<T> White()		{ return TVec3<T>(T(1   ), T(1   ), T(1   )); }
	LM_FORCE_INLINE static TVec3<T> Silver()	{ return TVec3<T>(T(0.75), T(0.75), T(0.75)); }
	LM_FORCE_INLINE static TVec3<T> Gray()		{ return TVec3<T>(T(0.5 ), T(0.5 ), T(0.5 )); }
	LM_FORCE_INLINE static TVec3<T> Black()		{ return TVec3<T>(T(0   ), T(0   ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Red()		{ return TVec3<T>(T(1   ), T(0   ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Maroon()	{ return TVec3<T>(T(0.5 ), T(0   ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Yellow()	{ return TVec3<T>(T(1   ), T(1   ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Olive()		{ return TVec3<T>(T(0.5 ), T(0.5 ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Green()		{ return TVec3<T>(T(0   ), T(1   ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Lime()		{ return TVec3<T>(T(0   ), T(0.5 ), T(0   )); }
	LM_FORCE_INLINE static TVec3<T> Aqua()		{ return TVec3<T>(T(0   ), T(1   ), T(1   )); }
	LM_FORCE_INLINE static TVec3<T> Teal()		{ return TVec3<T>(T(0   ), T(0.5 ), T(0.5 )); }
	LM_FORCE_INLINE static TVec3<T> Blue()		{ return TVec3<T>(T(0   ), T(0   ), T(1   )); }
	LM_FORCE_INLINE static TVec3<T> Navy()		{ return TVec3<T>(T(0   ), T(0   ), T(0.5 )); }
	LM_FORCE_INLINE static TVec3<T> Fuchsia()	{ return TVec3<T>(T(1   ), T(0   ), T(1   )); }
	LM_FORCE_INLINE static TVec3<T> Purple()	{ return TVec3<T>(T(0.5 ), T(0   ), T(0.5 )); }

};

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_MATH_COLORS_H