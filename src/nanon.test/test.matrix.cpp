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

#include "pch.h"
#include "base.math.h"
#include <nanon/math.h>

using namespace nanon;

NANON_TEST_NAMESPACE_BEGIN

template <typename T>
class MatrixTest : public MathTestBase<T>
{
public:

	MatrixTest()
	{
		m1 = TMat4<T>(
			T(1), T(2), T(3), T(4),
			T(5), T(6), T(7), T(8),
			T(9), T(10), T(11), T(12),
			T(13), T(14), T(15), T(16));

		m2 = TMat4<T>(
			T(1), T(5), T(9), T(13),
			T(2), T(6), T(10), T(14),
			T(3), T(7), T(11), T(15),
			T(4), T(8), T(12), T(16));

		m1m2 = TMat4<T>(
			T(276), T(304), T(332), T(360),
			T(304), T(336), T(368), T(400),
			T(332), T(368), T(404), T(440),
			T(360), T(400), T(440), T(480));
	}

protected:

	::testing::AssertionResult ExpectMat4Near(const TMat4<T>& expect, const TMat4<T>& actual)
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

protected:

	TMat4<T> zero, identity;
	TMat4<T> m1, m2, m3;
	TMat4<T> m1m2;

};

TYPED_TEST_CASE(MatrixTest, MathTestTypes);

TYPED_TEST(MatrixTest, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			T v(i*4+j+1);
			EXPECT_TRUE(ExpectNear(v, m1.v[i][j]));
		}
	}
}

TYPED_TEST(MatrixTest, Accessor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			T v(i*4+j+1);
			EXPECT_TRUE(ExpectNear(v, m1[i][j]));
		}
	}
}

TYPED_TEST(MatrixTest, Multiply)
{
	EXPECT_TRUE(ExpectMat4Near(m1m2, m1 * m2));
}

NANON_TEST_NAMESPACE_END