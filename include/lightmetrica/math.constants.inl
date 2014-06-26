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
	return InvTwoPi() * T(2);
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::InvTwoPi()
{
	return boost::math::constants::one_div_two_pi<T>();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Inf()
{
	return std::numeric_limits<T>::max();
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::Eps()
{
	return T(1e-7);
}

template <>
LM_FORCE_INLINE float TConstants<float>::Eps()
{
	return 1e-4f;
}

template <typename T>
LM_FORCE_INLINE T TConstants<T>::EpsLarge()
{
	return T(1e-5);
}

template <>
LM_FORCE_INLINE float TConstants<float>::EpsLarge()
{
	return 1e-3f;
}

template <typename T>
LM_FORCE_INLINE static T MachineEps()
{
	return std::numeric_limits<T>::epsilon();
}

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END