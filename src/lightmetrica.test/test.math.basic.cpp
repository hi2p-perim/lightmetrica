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

#include "pch.test.h"
#include <lightmetrica.test/base.math.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

template <typename T>
class MathBasicTest : public MathTestBase<T> {};

TYPED_TEST_CASE(MathBasicTest, MathTestTypes);

TYPED_TEST(MathBasicTest, MinMax)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(0), Math::Min(T(0), T(1))));
	EXPECT_TRUE(ExpectNear(T(1), Math::Max(T(0), T(1))));
}

TYPED_TEST(MathBasicTest, Clamp)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(0.5), Math::Clamp(T(0.5), T(0), T(1))));
	EXPECT_TRUE(ExpectNear(T(0), Math::Clamp(T(-0.5), T(0), T(1))));
	EXPECT_TRUE(ExpectNear(T(1), Math::Clamp(T(1.5), T(0), T(1))));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END