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

#ifndef __NANON_TEST_BASE_MATH_H__
#define __NANON_TEST_BASE_MATH_H__

#include "base.h"
#include <nanon/math.types.h>
#include <cmath>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace mp = boost::multiprecision;

NANON_TEST_NAMESPACE_BEGIN

//typedef mp::number<mp::cpp_dec_float<100>, mp::et_off> BigFloat;
typedef mp::cpp_dec_float_50 BigFloat;
typedef ::testing::Types<float, double, BigFloat> MathTestTypes;

template <typename T>
class MathTestBase : public TestBase {};

template <typename T>
NANON_FORCE_INLINE T Epsilon()
{
	return std::numeric_limits<T>::epsilon();
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectNear(const T& expected, const T& actual)
{
	T diff = std::abs(expected - actual);
	T epsilon = Epsilon<T>();
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure() << "Difference " << diff << " (epsilon " << epsilon << " )";
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectNear<BigFloat>(const BigFloat& expected, const BigFloat& actual)
{
	BigFloat diff = mp::abs(expected - actual);
	BigFloat epsilon = Epsilon<BigFloat>();
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure() << "Difference " << diff.str() << " (epsilon " << epsilon.str() << " )";
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectVec2Near(const nanon::Math::TVec2<T>& expect, const nanon::Math::TVec2<T>& actual)
{
	for (int i = 0; i < 2; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectVec3Near(const nanon::Math::TVec3<T>& expect, const nanon::Math::TVec3<T>& actual)
{
	for (int i = 0; i < 3; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectVec4Near(const nanon::Math::TVec4<T>& expect, const nanon::Math::TVec4<T>& actual)
{
	for (int i = 0; i < 4; i++)
	{
		auto result = ExpectNear(expect[i], actual[i]);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectMat3Near(const nanon::Math::TMat3<T>& expect, const nanon::Math::TMat3<T>& actual)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto result = ExpectNear(expect[i][j], actual[i][j]);
			if (!result)
			{
				return result;
			}
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
NANON_FORCE_INLINE ::testing::AssertionResult ExpectMat4Near(const nanon::Math::TMat4<T>& expect, const nanon::Math::TMat4<T>& actual)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			auto result = ExpectNear(expect[i][j], actual[i][j]);
			if (!result)
			{
				return result;
			}
		}
	}

	return ::testing::AssertionSuccess();
}

NANON_TEST_NAMESPACE_END

#endif // __NANON_TEST_BASE_MATH_H__