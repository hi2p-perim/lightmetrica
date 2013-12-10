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
class VectorTest : public MathTestBase<T>
{
public:

	VectorTest()
	{
		v1 = TVec4<T>(T(1), T(2), T(3), T(4));
	}

protected:

	::testing::AssertionResult ExpectVec4Near(const TVec4<T>& expect, const TVec4<T>& actual)
	{
		for (int i = 0; i < 4; i++)
		{
			T diff = std::abs(expect[i] - actual[i]);
			if (diff > Epsilon)
			{
				return ::testing::AssertionFailure()
					<< boost::str(boost::format("expect[%d] (%f) is not near to actual[%d] (%f) by %f (epsilon %f)")
						% i % expect[i] % i % actual[i] % diff % Epsilon) << std::endl;
			}
		}

		return ::testing::AssertionSuccess();
	}

protected:

	TVec4<T> v1;

};

TYPED_TEST_CASE(VectorTest, MathTestTypes);

TYPED_TEST(VectorTest, Constructor)
{
	typedef TypeParam T;
	EXPECT_NEAR(T(1), v1.x, Epsilon);
	EXPECT_NEAR(T(2), v1.y, Epsilon);
	EXPECT_NEAR(T(3), v1.z, Epsilon);
	EXPECT_NEAR(T(4), v1.w, Epsilon);
}

TYPED_TEST(VectorTest, Accessor)
{
	typedef TypeParam T;
	EXPECT_NEAR(T(1), v1[0], Epsilon);
	EXPECT_NEAR(T(2), v1[1], Epsilon);
	EXPECT_NEAR(T(3), v1[2], Epsilon);
	EXPECT_NEAR(T(4), v1[3], Epsilon);
}

NANON_TEST_NAMESPACE_END