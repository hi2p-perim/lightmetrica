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
#include <nanon/math.functions.h>

using namespace nanon;

NANON_TEST_NAMESPACE_BEGIN

template <typename T>
class MathTransformTest : public MathTestBase<T> {};

TYPED_TEST_CASE(MathTransformTest, MathTestTypes);

TYPED_TEST(MathTransformTest, Translate)
{
	typedef TypeParam T;

	Math::TVec4<T> v1(T(1), T(2), T(3), T(1));
	Math::TVec3<T> v2(T(3), T(2), T(1));
	Math::TVec4<T> expect(T(4), T(4), T(4), T(1));

	EXPECT_TRUE(ExpectVec4Near(expect, Math::Translate(v2) * v1));
}

TYPED_TEST(MathTransformTest, Rotate)
{
	typedef TypeParam T;

	Math::TVec4<T> v(T(1), T(0), T(0), T(1));
	Math::TVec3<T> axis(T(0), T(0), T(1));
	T angle(90);
	Math::TVec4<T> expect(T(0), T(1), T(0), T(1));
	
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Rotate(angle, axis) * v));
}

TYPED_TEST(MathTransformTest, Scale)
{
	typedef TypeParam T;

	Math::TVec4<T> v(T(1), T(2), T(3), T(1));
	Math::TVec3<T> scale(T(2));
	Math::TVec4<T> expect(T(2), T(4), T(6), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Scale(scale) * v));
}

//TYPED_TEST(MathTransformTest, LookAt)
//{
//
//}

//TYPED_TEST(MathTransformTest, Perspective)
//{
//
//}

NANON_TEST_NAMESPACE_END