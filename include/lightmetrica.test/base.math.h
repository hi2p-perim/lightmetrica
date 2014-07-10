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
#ifndef LIB_LIGHTMETRICA_TEST_BASE_MATH_H
#define LIB_LIGHTMETRICA_TEST_BASE_MATH_H

#include "base.h"
#ifndef LM_ENABLE_MULTI_PRECISION
#define LM_ENABLE_MULTI_PRECISION
#endif
#include <lightmetrica/math.types.h>
#include <lightmetrica/math.basic.h>
#include <lightmetrica/align.h>

namespace mp = boost::multiprecision;

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

typedef ::testing::Types<float, double, Math::BigFloat> MathTestTypes;

template <typename T>
class MathTestBase : public TestBase {};

template <typename T>
inline ::testing::AssertionResult ExpectNearRelative(const T& expected, const T& actual, const T& epsilon)
{
	T diff = Math::Abs(expected - actual) / Math::Abs(expected);
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected		<< ", "
			<< "Actual "	<< actual		<< ", "
			<< "Diff "		<< diff			<< ", "
			<< "Epsilon "	<< epsilon;
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectNear(const T& expected, const T& actual, const T& epsilon)
{
	T diff = Math::Abs(expected - actual);
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected		<< ", "
			<< "Actual "	<< actual		<< ", "
			<< "Diff "		<< diff			<< ", "
			<< "Epsilon "	<< epsilon;
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectNear(const T& expected, const T& actual)
{
	return ExpectNear(expected, actual, Math::TConstants<T>::EpsLarge());
}

template <>
inline ::testing::AssertionResult ExpectNear<Math::BigFloat>(const Math::BigFloat& expected, const Math::BigFloat& actual)
{
	Math::BigFloat diff = mp::abs(expected - actual);
	Math::BigFloat epsilon = Math::TConstants<Math::BigFloat>::EpsLarge();
	if (diff > epsilon)
	{
		return ::testing::AssertionFailure()
			<< "Expected "	<< expected.str()	<< ", "
			<< "Actual "	<< actual.str()		<< ", "
			<< "Diff "		<< diff.str()		<< ", "
			<< "Epsilon "	<< epsilon.str();
	}
	else
	{
		return ::testing::AssertionSuccess();
	}
}

template <typename T>
inline ::testing::AssertionResult ExpectVec2Near(const Math::TVec2<T>& expect, const Math::TVec2<T>& actual)
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
inline ::testing::AssertionResult ExpectVec3Near(const Math::TVec3<T>& expect, const Math::TVec3<T>& actual, const T& epsilon)
{
	for (int i = 0; i < 3; i++)
	{
		auto result = ExpectNear(expect[i], actual[i], epsilon);
		if (!result)
		{
			return result;
		}
	}

	return ::testing::AssertionSuccess();
}

template <typename T>
inline ::testing::AssertionResult ExpectVec3Near(const Math::TVec3<T>& expect, const Math::TVec3<T>& actual)
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
inline ::testing::AssertionResult ExpectVec4Near(const Math::TVec4<T>& expect, const Math::TVec4<T>& actual)
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
inline ::testing::AssertionResult ExpectMat3Near(const Math::TMat3<T>& expect, const Math::TMat3<T>& actual)
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
inline ::testing::AssertionResult ExpectMat4Near(const Math::TMat4<T>& expect, const Math::TMat4<T>& actual)
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

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_BASE_MATH_H
