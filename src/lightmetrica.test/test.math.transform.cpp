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
#include <lightmetrica/math.transform.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

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

TYPED_TEST(MathTransformTest, LookAt)
{
	typedef TypeParam T;

	auto V1 = Math::LookAt(
		Math::TVec3<T>(T(0), T(1), T(0)),
		Math::TVec3<T>(T(0)),
		Math::TVec3<T>(T(0), T(0), T(1)));

	auto V2 = Math::LookAt(
		Math::TVec3<T>(T(1)),
		Math::TVec3<T>(T(0)),
		Math::TVec3<T>(T(0), T(0), T(1)));

	Math::TVec4<T> t;
	Math::TVec4<T> expect;

	// (0, 0, 0) in world coords. -> (0, 0, -1) in eye coords.
	t = V1 * Math::TVec4<T>(T(0), T(0), T(0), T(1));
	expect = Math::TVec4<T>(T(0), T(0), T(-1), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, t));

	// (0, 0, 0) in world coords. -> (0, 0, -sqrt(3)) in eye coords.
	t = V2 * Math::TVec4<T>(T(0), T(0), T(0), T(1));
	expect = Math::TVec4<T>(T(0), T(0), -Math::Sqrt(T(3)), T(1));
	EXPECT_TRUE(ExpectVec4Near(expect, t));
}

TYPED_TEST(MathTransformTest, Perspective)
{
	typedef TypeParam T;

	T fovy(90), aspect(1.5), zNear(1), zFar(1000);
	auto P = Math::Perspective(fovy, aspect, zNear, zFar);

	Math::TVec4<T> t;
	Math::TVec3<T> expect;

	// (0, 0, -1) -> (0, 0, -1) in NDC
	t = P * Math::TVec4<T>(T(0), T(0), T(-1), T(1));
	expect = Math::TVec3<T>(T(0), T(0), T(-1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));

	// (0, 0, -1000) -> (0, 0, 1) in NDC
	t = P * Math::TVec4<T>(T(0), T(0), T(-1000), T(1));
	expect = Math::TVec3<T>(T(0), T(0), T(1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));
	
	// (1.5, 1, -1) -> (1, 1, -1) in NDC
	t = P * Math::TVec4<T>(T(1.5), T(1), T(-1), T(1));
	expect = Math::TVec3<T>(T(1), T(1), T(-1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));

	// (-1500, -1000, -1000) -> (-1, -1, 1) in NDC
	t = P * Math::TVec4<T>(T(-1500), T(-1000), T(-1000), T(1));
	expect = Math::TVec3<T>(T(-1), T(-1), T(1));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::TVec3<T>(t) / t.w));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END