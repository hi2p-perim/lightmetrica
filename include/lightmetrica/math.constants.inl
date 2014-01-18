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

#include "math.constants.h"
#include <limits>
#include <boost/math/constants/constants.hpp>

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Zero()
{
	return T(0);
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Pi()
{
	return boost::math::constants::pi<T>();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::InvPi()
{
	return T(0.31830988618379067154);
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::InvTwoPi()
{
	return boost::math::constants::one_div_two_pi()<T>();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Inf()
{
	return std::numeric_limits<T>::infinity();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Eps()
{
	return std::numeric_limits<T>::epsilon();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::EpsLarge()
{
	return T(1e-10);
}

template <>
LM_FORCE_INLINE float TConstants<float>::EpsLarge()
{
	return 1e-3f;
}

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END