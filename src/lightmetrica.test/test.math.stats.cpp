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

#include "pch.h"
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/math.stats.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

template <typename T>
class MathStatsTest : public MathTestBase<T> {};

TYPED_TEST_CASE(MathStatsTest, MathTestTypes);

TYPED_TEST(MathStatsTest, UniformConcentricDiskSample)
{
	typedef TypeParam T;

	const int Count = 5;

	const T source[] = {
		T(0.5), T(0.5),
		T(1), T(0.5),
		T(0.5), T(1),
		T(0), T(0.5),
		T(0.5), T(0)
	};

	const T expect[] = {
		T(0), T(0),
		T(1), T(0),
		T(0), T(1),
		T(-1), T(0),
		T(0), T(-1)
	};

	for (int i = 0; i < Count; i++)
	{
		EXPECT_TRUE(ExpectVec2Near(
			Math::TVec2<T>(expect[2*i], expect[2*i+1]),
			Math::UniformConcentricDiskSample(Math::TVec2<T>(source[2*i], source[2*i+1]))));
	}
}

TYPED_TEST(MathStatsTest, CosineSampleHemisphere)
{
	typedef TypeParam T;

	const int Count = 1;

	const T source[] = {
		T(0.5), T(0.5),
		T(1), T(0.5),
		T(0.75), T(0.75),
	};

	const T s2 = Math::Sqrt<T>(T(2));
	const T s3 = Math::Sqrt<T>(T(3));
	const T expect[] = {
		T(0), T(0), T(1),
		T(1), T(0), T(0),
		T(s2 / T(4)), T(s2 / T(4)), T(s3 / T(2)),
	};

	for (int i = 0; i < Count; i++)
	{
		EXPECT_TRUE(ExpectVec3Near(
			Math::TVec3<T>(expect[3*i], expect[3*i+1], expect[3*i+2]),
			Math::CosineSampleHemisphere(Math::TVec2<T>(source[2*i], source[2*i+1]))));
	}
}

//TYPED_TEST(MathStatsTest, UniformSampleHemisphere)
//{
//	typedef TypeParam T;
//	FAIL() << "Not implemented";
//}

//TYPED_TEST(MathStatsTest, UniformSampleSphere)
//{
//	typedef TypeParam T;
//	FAIL() << "Not implemented";
//}

//TYPED_TEST(MathStatsTest, UniformSampleTriangle)
//{
//	typedef TypeParam T;
//	FAIL() << "Not implemented";
//}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END