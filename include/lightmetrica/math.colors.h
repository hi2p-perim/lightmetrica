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
#ifndef __LIB_LIGHTMETRICA_MATH_COLORS_H__
#define __LIB_LIGHTMETRICA_MATH_COLORS_H__

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

#endif // __LIB_LIGHTMETRICA_MATH_COLORS_H__