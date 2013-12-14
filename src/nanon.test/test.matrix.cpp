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

		m1s2 = TMat4<T>(
			T(2), T(4), T(6), T(8),
			T(10), T(12), T(14), T(16),
			T(18), T(20), T(22), T(24),
			T(26), T(28), T(30), T(32));

		m1m2 = TMat4<T>(
			T(276), T(304), T(332), T(360),
			T(304), T(336), T(368), T(400),
			T(332), T(368), T(404), T(440),
			T(360), T(400), T(440), T(480));

		v1 = TVec4<T>(T(4), T(3), T(2), T(1));
		m1v1 = TVec4<T>(T(50), T(60), T(70), T(80));
	}

protected:

	TMat4<T> zero, identity;
	TMat4<T> m1, m2, m3;
	TMat4<T> m1s2, m1m2;
	TVec4<T> v1, m1v1;

};

TYPED_TEST_CASE(MatrixTest, MathTestTypes);

TYPED_TEST(MatrixTest, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), m1.v[i][j]));
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
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), m1[i][j]));
		}
	}
}

TYPED_TEST(MatrixTest, Multiply)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectMat4Near(m1s2, m1 * T(2)));
	EXPECT_TRUE(ExpectMat4Near(m1s2, T(2) * m1));
	EXPECT_TRUE(ExpectVec4Near(m1v1, m1 * v1));
	EXPECT_TRUE(ExpectMat4Near(m1m2, m1 * m2));
}

NANON_TEST_NAMESPACE_END