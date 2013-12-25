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

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

template <typename T>
class MathVector2Test : public MathTestBase<T>
{
public:

	MathVector2Test()
	{
		v1 = Math::TVec2<T>(T(1), T(2));
		v2 = Math::TVec2<T>(T(4), T(3));
	}

protected:

	Math::TVec2<T> v1, v2;

};

TYPED_TEST_CASE(MathVector2Test, MathTestTypes);

TYPED_TEST(MathVector2Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
}

TYPED_TEST(MathVector2Test, Conversion)
{
	typedef TypeParam T;
	Math::TVec3<T> t1(T(1), T(2), T(3));
	Math::TVec4<T> t2(T(1), T(2), T(3), T(4));
	EXPECT_TRUE(ExpectVec2Near(v1, Math::TVec2<T>(t1)));
	EXPECT_TRUE(ExpectVec2Near(v1, Math::TVec2<T>(t2)));
}

TYPED_TEST(MathVector2Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
}

TYPED_TEST(MathVector2Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec2<T> v1plusv2(T(5));
	Math::TVec2<T> v1minusv2(T(-3), T(-1));
	EXPECT_TRUE(ExpectVec2Near(v1plusv2, v1 + v2));
	EXPECT_TRUE(ExpectVec2Near(v1plusv2, v2 + v1));
	EXPECT_TRUE(ExpectVec2Near(v1minusv2, v1 - v2));
}

TYPED_TEST(MathVector2Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec2<T> v1s2(T(2), T(4));
	Math::TVec2<T> v1v2(T(4), T(6));
	EXPECT_TRUE(ExpectVec2Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec2Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec2Near(v1v2, v1 * v2));
	EXPECT_TRUE(ExpectVec2Near(v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec2Near(v1, v1v2 / v2));
}

TYPED_TEST(MathVector2Test, Unary)
{
	typedef TypeParam T;
	Math::TVec2<T> expect(T(-1), T(-2));
	EXPECT_TRUE(ExpectVec2Near(expect, -v1));
}

TYPED_TEST(MathVector2Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(5), Math::Length(v2)));
	EXPECT_TRUE(ExpectNear(T(25), Math::Length2(v2)));
}

TYPED_TEST(MathVector2Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec2<T> expect(T(0.8), T(0.6));
	EXPECT_TRUE(ExpectVec2Near(expect, Math::Normalize(v2)));
}

TYPED_TEST(MathVector2Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(10), Math::Dot(v1, v2)));
}

// --------------------------------------------------------------------------------

template <typename T>
class MathVector3Test : public MathTestBase<T>
{
public:

	MathVector3Test()
	{
		v1 = Math::TVec3<T>(T(1), T(2), T(3));
		v2 = Math::TVec3<T>(T(4), T(3), T(2));
		v3 = Math::TVec3<T>(T(2), T(2), T(1));
	}

protected:

	Math::TVec3<T> v1, v2, v3;

};

TYPED_TEST_CASE(MathVector3Test, MathTestTypes);

TYPED_TEST(MathVector3Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
	EXPECT_TRUE(ExpectNear(T(3), v1.z));
}

TYPED_TEST(MathVector3Test, Conversion)
{
	typedef TypeParam T;

	Math::TVec2<T> t1(T(1), T(2));
	Math::TVec3<T> a1(T(1), T(2), T(0));
	EXPECT_TRUE(ExpectVec3Near(a1, Math::TVec3<T>(t1)));

	Math::TVec4<T> t2(T(1), T(2), T(3), T(4));
	EXPECT_TRUE(ExpectVec3Near(v1, Math::TVec3<T>(t2)));
}

TYPED_TEST(MathVector3Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), v1[2]));
}

TYPED_TEST(MathVector3Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec3<T> v1plusv2(T(5));
	Math::TVec3<T> v1minusv2(T(-3), T(-1), T(1));
	EXPECT_TRUE(ExpectVec3Near(v1plusv2, v1 + v2));
	EXPECT_TRUE(ExpectVec3Near(v1plusv2, v2 + v1));
	EXPECT_TRUE(ExpectVec3Near(v1minusv2, v1 - v2));
}

TYPED_TEST(MathVector3Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec3<T> v1s2(T(2), T(4), T(6));
	Math::TVec3<T> v1v2(T(4), T(6), T(6));
	EXPECT_TRUE(ExpectVec3Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec3Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec3Near(v1v2, v1 * v2));
	EXPECT_TRUE(ExpectVec3Near(v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec3Near(v1, v1v2 / v2));
}

TYPED_TEST(MathVector3Test, Unary)
{
	typedef TypeParam T;
	Math::TVec3<T> expect(T(-1), T(-2), T(-3));
	EXPECT_TRUE(ExpectVec3Near(expect, -v1));
}

TYPED_TEST(MathVector3Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(3), Math::Length(v3)));
	EXPECT_TRUE(ExpectNear(T(9), Math::Length2(v3)));
}

TYPED_TEST(MathVector3Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec3<T> expect = v3 / T(3);
	EXPECT_TRUE(ExpectVec3Near(expect, Math::Normalize(v3)));
}

TYPED_TEST(MathVector3Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(16), Math::Dot(v1, v2)));
}

TYPED_TEST(MathVector3Test, Cross)
{
	typedef TypeParam T;
	Math::TVec3<T> expect(T(-5), T(10), T(-5));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::Cross(v1, v2)));
}


// --------------------------------------------------------------------------------

template <typename T>
class MathVector4Test : public MathTestBase<T>
{
public:

	MathVector4Test()
	{
		v1 = Math::TVec4<T>(T(1), T(2), T(3), T(4));
		v2 = Math::TVec4<T>(T(4), T(3), T(2), T(1));
		v3 = Math::TVec4<T>(T(1), T(2), T(0), T(2));
	}

protected:

	Math::TVec4<T> v1, v2, v3;

};

TYPED_TEST_CASE(MathVector4Test, MathTestTypes);

TYPED_TEST(MathVector4Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1.x));
	EXPECT_TRUE(ExpectNear(T(2), v1.y));
	EXPECT_TRUE(ExpectNear(T(3), v1.z));
	EXPECT_TRUE(ExpectNear(T(4), v1.w));
}

TYPED_TEST(MathVector4Test, Conversion)
{
	typedef TypeParam T;

	Math::TVec2<T> t1(T(1), T(2));
	Math::TVec4<T> a1(T(1), T(2), T(0), T(0));
	EXPECT_TRUE(ExpectVec4Near(a1, Math::TVec4<T>(t1)));

	Math::TVec3<T> t2(T(1), T(2), T(3));
	Math::TVec4<T> a2(T(1), T(2), T(3), T(0));
	EXPECT_TRUE(ExpectVec4Near(a2, Math::TVec4<T>(t2)));

	EXPECT_TRUE(ExpectVec4Near(v1, Math::TVec4<T>(t2, T(4))));
}

TYPED_TEST(MathVector4Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), v1[2]));
	EXPECT_TRUE(ExpectNear(T(4), v1[3]));
}

TYPED_TEST(MathVector4Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec4<T> v1plusv2(T(5));
	Math::TVec4<T> v1minusv2(T(-3), T(-1), T(1), T(3));
	EXPECT_TRUE(ExpectVec4Near(v1plusv2, v1 + v2));
	EXPECT_TRUE(ExpectVec4Near(v1plusv2, v2 + v1));
	EXPECT_TRUE(ExpectVec4Near(v1minusv2, v1 - v2));
}

TYPED_TEST(MathVector4Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec4<T> v1s2(T(2), T(4), T(6), T(8));
	Math::TVec4<T> v1v2(T(4), T(6), T(6), T(4));
	EXPECT_TRUE(ExpectVec4Near(v1s2, v1 * T(2)));
	EXPECT_TRUE(ExpectVec4Near(v1s2, T(2) * v1));
	EXPECT_TRUE(ExpectVec4Near(v1v2, v1 * v2));
	EXPECT_TRUE(ExpectVec4Near(v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec4Near(v1, v1v2 / v2));
}

TYPED_TEST(MathVector4Test, Unary)
{
	typedef TypeParam T;
	Math::TVec4<T> expect(T(-1), T(-2), T(-3), T(-4));
	EXPECT_TRUE(ExpectVec4Near(expect, -v1));
}

TYPED_TEST(MathVector4Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(3), Math::Length(v3)));
	EXPECT_TRUE(ExpectNear(T(9), Math::Length2(v3)));
}

TYPED_TEST(MathVector4Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec4<T> expect = v3 / T(3);
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Normalize(v3)));
}

TYPED_TEST(MathVector4Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(20), Math::Dot(v1, v2)));
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END