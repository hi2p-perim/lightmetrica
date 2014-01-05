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

#pragma once
#ifndef __LIB_NANON_MATH_CONSTANTS_H__
#define __LIB_NANON_MATH_CONSTANTS_H__

#include "math.common.h"
#include <limits>
#include <boost/math/constants/constants.hpp>

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

template <typename T>
class TConstants
{
public:

	TConstants();
	~TConstants();

	NANON_DISABLE_COPY_AND_MOVE(TConstants);

public:

	static const T Zero;
	static const T Pi;
	static const T InvPi;
	static const T InvTwoPi;
	static const T Inf;
	static const T Eps;
	static const T EpsLarge;

};

template <typename T> const T TConstants<T>::Zero = T(0);
template <typename T> const T TConstants<T>::Pi = boost::math::constants::pi<T>();
template <typename T> const T TConstants<T>::InvPi = T(0.31830988618379067154);
template <typename T> const T TConstants<T>::InvTwoPi = boost::math::constants::one_div_two_pi()<T>();
template <typename T> const T TConstants<T>::Inf = std::numeric_limits<T>::infinity();
template <typename T> const T TConstants<T>::Eps = std::numeric_limits<T>::epsilon();
template <typename T> const T TConstants<T>::EpsLarge = T(1e-10);
template <> const float TConstants<float>::EpsLarge = 1e-3f;

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#endif // __LIB_NANON_MATH_CONSTANTS_H__