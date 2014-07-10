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