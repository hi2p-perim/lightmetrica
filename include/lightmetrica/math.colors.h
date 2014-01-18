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

#include "math.common.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

/*!
	Useful color constants for TVec3.
	For internal use.
*/
template <typename T>
class TColors
{
public:

	TColors();
	~TColors();

	LM_DISABLE_COPY_AND_MOVE(TColors);

public:

	static const TVec3<T> White;
	static const TVec3<T> Silver;
	static const TVec3<T> Gray;
	static const TVec3<T> Black;
	static const TVec3<T> Red;
	static const TVec3<T> Maroon;
	static const TVec3<T> Yellow;
	static const TVec3<T> Olive;
	static const TVec3<T> Green;
	static const TVec3<T> Lime;
	static const TVec3<T> Aqua;
	static const TVec3<T> Teal;
	static const TVec3<T> Blue;
	static const TVec3<T> Navy;
	static const TVec3<T> Fuchsia;
	static const TVec3<T> Purple;

};

template <typename T> const TVec3<T> TColors<T>::White		= TVec3<T>(T(1   ), T(1   ), T(1   ));
template <typename T> const TVec3<T> TColors<T>::Silver		= TVec3<T>(T(0.75), T(0.75), T(0.75));
template <typename T> const TVec3<T> TColors<T>::Gray		= TVec3<T>(T(0.5 ), T(0.5 ), T(0.5 ));
template <typename T> const TVec3<T> TColors<T>::Black		= TVec3<T>(T(0   ), T(0   ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Red		= TVec3<T>(T(1   ), T(0   ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Maroon		= TVec3<T>(T(0.5 ), T(0   ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Yellow		= TVec3<T>(T(1   ), T(1   ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Olive		= TVec3<T>(T(0.5 ), T(0.5 ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Green		= TVec3<T>(T(0   ), T(1   ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Lime		= TVec3<T>(T(0   ), T(0.5 ), T(0   ));
template <typename T> const TVec3<T> TColors<T>::Aqua		= TVec3<T>(T(0   ), T(1   ), T(1   ));
template <typename T> const TVec3<T> TColors<T>::Teal		= TVec3<T>(T(0   ), T(0.5 ), T(0.5 ));
template <typename T> const TVec3<T> TColors<T>::Blue		= TVec3<T>(T(0   ), T(0   ), T(1   ));
template <typename T> const TVec3<T> TColors<T>::Navy		= TVec3<T>(T(0   ), T(0   ), T(0.5 ));
template <typename T> const TVec3<T> TColors<T>::Fuchsia	= TVec3<T>(T(1   ), T(0   ), T(1   ));
template <typename T> const TVec3<T> TColors<T>::Purple		= TVec3<T>(T(0.5 ), T(0   ), T(0.5 ));

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_MATH_COLORS_H__