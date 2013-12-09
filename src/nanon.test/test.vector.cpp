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
class VectorTest : public MathTestBase<T> {};

TYPED_TEST_CASE(VectorTest, MathTestTypes);

TYPED_TEST(VectorTest, Constructor)
{
	auto x = TypeParam(1);
	auto y = TypeParam(2);
	auto z = TypeParam(3);
	auto w = TypeParam(4);
	auto v = TVec4<TypeParam>(x, y, z, w);
	EXPECT_NEAR(x, v.x, Epsilon);
	EXPECT_NEAR(y, v.y, Epsilon);
	EXPECT_NEAR(z, v.z, Epsilon);
	EXPECT_NEAR(w, v.w, Epsilon);
}

NANON_TEST_NAMESPACE_END