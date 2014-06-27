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

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

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
	EXPECT_TRUE(ExpectNear(T(1), this->v1.x));
	EXPECT_TRUE(ExpectNear(T(2), this->v1.y));
}

TYPED_TEST(MathVector2Test, Conversion)
{
	typedef TypeParam T;
	Math::TVec3<T> t1(T(1), T(2), T(3));
	Math::TVec4<T> t2(T(1), T(2), T(3), T(4));
	EXPECT_TRUE(ExpectVec2Near(this->v1, Math::TVec2<T>(t1)));
	EXPECT_TRUE(ExpectVec2Near(this->v1, Math::TVec2<T>(t2)));
}

TYPED_TEST(MathVector2Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), this->v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), this->v1[1]));
}

TYPED_TEST(MathVector2Test, AddSubtractAssign)
{
	typedef TypeParam T;
	Math::TVec2<T> t;
	Math::TVec2<T> v1plusv2(T(5));
	Math::TVec2<T> v1minusv2(T(-3), T(-1));

	t = this->v1; t += this->v2;
	EXPECT_TRUE(ExpectVec2Near(v1plusv2, t));

	t = this->v1; t -= this->v2;
	EXPECT_TRUE(ExpectVec2Near(v1minusv2, t));
}

TYPED_TEST(MathVector2Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TVec2<T> t;
	Math::TVec2<T> v1s2(T(2), T(4));
	Math::TVec2<T> v1v2(T(4), T(6));

	t = this->v1; t *= T(2);
	EXPECT_TRUE(ExpectVec2Near(v1s2, t));

	t = this->v1; t *= this->v2;
	EXPECT_TRUE(ExpectVec2Near(v1v2, this->v1 * this->v2));

	t = v1s2; t /= T(2);
	EXPECT_TRUE(ExpectVec2Near(this->v1, v1s2 / T(2)));

	t = v1v2; t /= this->v2;
	EXPECT_TRUE(ExpectVec2Near(this->v1, v1v2 / this->v2));
}

TYPED_TEST(MathVector2Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec2<T> v1plusv2(T(5));
	Math::TVec2<T> v1minusv2(T(-3), T(-1));
	EXPECT_TRUE(ExpectVec2Near(v1plusv2, this->v1 + this->v2));
	EXPECT_TRUE(ExpectVec2Near(v1plusv2, this->v2 + this->v1));
	EXPECT_TRUE(ExpectVec2Near(v1minusv2, this->v1 - this->v2));
}

TYPED_TEST(MathVector2Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec2<T> v1s2(T(2), T(4));
	Math::TVec2<T> v1v2(T(4), T(6));
	EXPECT_TRUE(ExpectVec2Near(v1s2, this->v1 * T(2)));
	EXPECT_TRUE(ExpectVec2Near(v1s2, T(2) * this->v1));
	EXPECT_TRUE(ExpectVec2Near(v1v2, this->v1 * this->v2));
	EXPECT_TRUE(ExpectVec2Near(this->v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec2Near(this->v1, v1v2 / this->v2));
}

TYPED_TEST(MathVector2Test, Unary)
{
	typedef TypeParam T;
	Math::TVec2<T> expect(T(-1), T(-2));
	EXPECT_TRUE(ExpectVec2Near(expect, -this->v1));
}

TYPED_TEST(MathVector2Test, BinaryOperator)
{
	typedef TypeParam T;
	EXPECT_TRUE(this->v1 == this->v1);
	EXPECT_TRUE(this->v1 != this->v2);
}

TYPED_TEST(MathVector2Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(5), Math::Length(this->v2)));
	EXPECT_TRUE(ExpectNear(T(25), Math::Length2(this->v2)));
}

TYPED_TEST(MathVector2Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec2<T> expect(T(0.8), T(0.6));
	EXPECT_TRUE(ExpectVec2Near(expect, Math::Normalize(this->v2)));
}

TYPED_TEST(MathVector2Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(10), Math::Dot(this->v1, this->v2)));
}

TYPED_TEST(MathVector2Test, MinMax)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec2Near(this->v1, Math::Min(this->v1, this->v2)));
	EXPECT_TRUE(ExpectVec2Near(this->v2, Math::Max(this->v1, this->v2)));
}

// --------------------------------------------------------------------------------

template <typename T>
struct MathVector3Test_Data : public Aligned<std::alignment_of<Math::TVec3<T>>::value>
{

	MathVector3Test_Data()
	{
		v1 = Math::TVec3<T>(T(1), T(2), T(3));
		v2 = Math::TVec3<T>(T(4), T(3), T(2));
		v3 = Math::TVec3<T>(T(2), T(2), T(1));
	}

	Math::TVec3<T> v1, v2, v3;

};

template <typename T>
class MathVector3Test : public MathTestBase<T>
{
public:

	MathVector3Test()
	{
	    d = new MathVector3Test_Data<T>();
    }
	virtual ~MathVector3Test() { LM_SAFE_DELETE(d); }

protected:

	MathVector3Test_Data<T>* d;

};

TYPED_TEST_CASE(MathVector3Test, MathTestTypes);

TYPED_TEST(MathVector3Test, Alignment)
{
	EXPECT_TRUE(is_aligned(this->d, std::alignment_of<MathVector3Test_Data<TypeParam>>::value));
}

TYPED_TEST(MathVector3Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), this->d->v1.x));
	EXPECT_TRUE(ExpectNear(T(2), this->d->v1.y));
	EXPECT_TRUE(ExpectNear(T(3), this->d->v1.z));
}

TYPED_TEST(MathVector3Test, Conversion)
{
	typedef TypeParam T;

	Math::TVec2<T> t1(T(1), T(2));
	Math::TVec3<T> a1(T(1), T(2), T(0));
	EXPECT_TRUE(ExpectVec3Near(a1, Math::TVec3<T>(t1)));

	Math::TVec4<T> t2(T(1), T(2), T(3), T(4));
	EXPECT_TRUE(ExpectVec3Near(this->d->v1, Math::TVec3<T>(t2)));

	EXPECT_TRUE(ExpectVec3Near(this->d->v1, Math::TVec3<T>(t1, T(3))));
}

TYPED_TEST(MathVector3Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), this->d->v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), this->d->v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), this->d->v1[2]));
}

TYPED_TEST(MathVector3Test, AddSubtractAssign)
{
	typedef TypeParam T;
	Math::TVec3<T> t;
	Math::TVec3<T> v1plusv2(T(5));
	Math::TVec3<T> v1minusv2(T(-3), T(-1), T(1));

	t = this->d->v1; t += this->d->v2;
	EXPECT_TRUE(ExpectVec3Near(v1plusv2, t));

	t = this->d->v1; t -= this->d->v2;
	EXPECT_TRUE(ExpectVec3Near(v1minusv2, t));
}

TYPED_TEST(MathVector3Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TVec3<T> t;
	Math::TVec3<T> v1s2(T(2), T(4), T(6));
	Math::TVec3<T> v1v2(T(4), T(6), T(6));

	t = this->d->v1; t *= T(2);
	EXPECT_TRUE(ExpectVec3Near(v1s2, t));

	t = this->d->v1; t *= this->d->v2;
	EXPECT_TRUE(ExpectVec3Near(v1v2, this->d->v1 * this->d->v2));

	t = v1s2; t /= T(2);
	EXPECT_TRUE(ExpectVec3Near(this->d->v1, v1s2 / T(2)));

	t = v1v2;
	t /= this->d->v2;
	EXPECT_TRUE(ExpectVec3Near(this->d->v1, v1v2 / this->d->v2));
}

TYPED_TEST(MathVector3Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec3<T> v1plusv2(T(5));
	Math::TVec3<T> v1minusv2(T(-3), T(-1), T(1));
	EXPECT_TRUE(ExpectVec3Near(v1plusv2, this->d->v1 + this->d->v2));
	EXPECT_TRUE(ExpectVec3Near(v1plusv2, this->d->v2 + this->d->v1));
	EXPECT_TRUE(ExpectVec3Near(v1minusv2, this->d->v1 - this->d->v2));
}

TYPED_TEST(MathVector3Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec3<T> v1s2(T(2), T(4), T(6));
	Math::TVec3<T> v1v2(T(4), T(6), T(6));
	EXPECT_TRUE(ExpectVec3Near(v1s2, this->d->v1 * T(2)));
	EXPECT_TRUE(ExpectVec3Near(v1s2, T(2) * this->d->v1));
	EXPECT_TRUE(ExpectVec3Near(v1v2, this->d->v1 * this->d->v2));
	EXPECT_TRUE(ExpectVec3Near(this->d->v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec3Near(this->d->v1, v1v2 / this->d->v2));
}

TYPED_TEST(MathVector3Test, Unary)
{
	typedef TypeParam T;
	Math::TVec3<T> expect(T(-1), T(-2), T(-3));
	EXPECT_TRUE(ExpectVec3Near(expect, -this->d->v1));
}

TYPED_TEST(MathVector3Test, BinaryOperator)
{
	typedef TypeParam T;
	EXPECT_TRUE(this->d->v1 == this->d->v1);
	EXPECT_TRUE(this->d->v1 != this->d->v2);
}

TYPED_TEST(MathVector3Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(3), Math::Length(this->d->v3)));
	EXPECT_TRUE(ExpectNear(T(9), Math::Length2(this->d->v3)));
}

TYPED_TEST(MathVector3Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec3<T> expect = this->d->v3 / T(3);
	EXPECT_TRUE(ExpectVec3Near(expect, Math::Normalize(this->d->v3)));
}

TYPED_TEST(MathVector3Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(16), Math::Dot(this->d->v1, this->d->v2)));
}

TYPED_TEST(MathVector3Test, Cross)
{
	typedef TypeParam T;
	Math::TVec3<T> expect(T(-5), T(10), T(-5));
	EXPECT_TRUE(ExpectVec3Near(expect, Math::Cross(this->d->v1, this->d->v2)));
}

TYPED_TEST(MathVector3Test, LInfinityNorm)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(3), T(2), T(1)))));
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(-3), T(2), T(1)))));
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(-3), T(-2), T(1)))));
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(-3), T(-2), T(-1)))));
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(1), T(2), T(3)))));
	EXPECT_TRUE(ExpectNear(T(3), Math::LInfinityNorm(Math::TVec3<T>(T(1), T(3), T(2)))));

	// This is a little tricky case
	// Fourth component of Vec3 is hidden when SIMD version of Vec3 is used
	EXPECT_TRUE(ExpectNear(T(1), Math::LInfinityNorm(Math::TVec3<T>(Math::TVec4<T>(T(1), T(1), T(1), T(2))))));
}

TYPED_TEST(MathVector3Test, MinMax)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec3Near(Math::TVec3<T>(T(1), T(2), T(2)), Math::Min(this->d->v1, this->d->v2)));
	EXPECT_TRUE(ExpectVec3Near(Math::TVec3<T>(T(4), T(3), T(3)), Math::Max(this->d->v1, this->d->v2)));
}

TYPED_TEST(MathVector3Test, IsZero)
{
	typedef TypeParam T;
	EXPECT_TRUE(Math::IsZero(Math::TVec3<T>(T(0))));
	EXPECT_FALSE(Math::IsZero(Math::TVec3<T>(T(1))));
}

// --------------------------------------------------------------------------------

template <typename T>
struct MathVector4Test_Data : public Aligned<std::alignment_of<Math::TVec4<T>>::value>
{

	MathVector4Test_Data()
	{
		v1 = Math::TVec4<T>(T(1), T(2), T(3), T(4));
		v2 = Math::TVec4<T>(T(4), T(3), T(2), T(1));
		v3 = Math::TVec4<T>(T(1), T(2), T(0), T(2));
	}

	Math::TVec4<T> v1, v2, v3;

};

template <typename T>
class MathVector4Test : public MathTestBase<T>
{
public:

	MathVector4Test() { d = new MathVector4Test_Data<T>(); }
	virtual ~MathVector4Test() { LM_SAFE_DELETE(d); }

protected:

	MathVector4Test_Data<T>* d;

};

TYPED_TEST_CASE(MathVector4Test, MathTestTypes);

TYPED_TEST(MathVector4Test, Alignment)
{
	EXPECT_TRUE(is_aligned(this->d, std::alignment_of<MathVector4Test_Data<TypeParam>>::value));
}

TYPED_TEST(MathVector4Test, Constructor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), this->d->v1.x));
	EXPECT_TRUE(ExpectNear(T(2), this->d->v1.y));
	EXPECT_TRUE(ExpectNear(T(3), this->d->v1.z));
	EXPECT_TRUE(ExpectNear(T(4), this->d->v1.w));
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

	EXPECT_TRUE(ExpectVec4Near(this->d->v1, Math::TVec4<T>(t2, T(4))));
}

TYPED_TEST(MathVector4Test, Accessor)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(1), this->d->v1[0]));
	EXPECT_TRUE(ExpectNear(T(2), this->d->v1[1]));
	EXPECT_TRUE(ExpectNear(T(3), this->d->v1[2]));
	EXPECT_TRUE(ExpectNear(T(4), this->d->v1[3]));
}

TYPED_TEST(MathVector4Test, AddSubtractAssign)
{
	typedef TypeParam T;
	Math::TVec4<T> t;
	Math::TVec4<T> v1plusv2(T(5));
	Math::TVec4<T> v1minusv2(T(-3), T(-1), T(1), T(3));

	t = this->d->v1; t += this->d->v2;
	EXPECT_TRUE(ExpectVec4Near(v1plusv2, t));

	t = this->d->v1; t -= this->d->v2;
	EXPECT_TRUE(ExpectVec4Near(v1minusv2, t));
}

TYPED_TEST(MathVector4Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TVec4<T> t;
	Math::TVec4<T> v1s2(T(2), T(4), T(6), T(8));
	Math::TVec4<T> v1v2(T(4), T(6), T(6), T(4));

	t = this->d->v1; t *= T(2);
	EXPECT_TRUE(ExpectVec4Near(v1s2, t));

	t = this->d->v1; t *= this->d->v2;
	EXPECT_TRUE(ExpectVec4Near(v1v2, this->d->v1 * this->d->v2));

	t = v1s2; t /= T(2);
	EXPECT_TRUE(ExpectVec4Near(this->d->v1, v1s2 / T(2)));

	t = v1v2; t /= this->d->v2;
	EXPECT_TRUE(ExpectVec4Near(this->d->v1, v1v2 / this->d->v2));
}

TYPED_TEST(MathVector4Test, AddSubtract)
{
	typedef TypeParam T;
	Math::TVec4<T> v1plusv2(T(5));
	Math::TVec4<T> v1minusv2(T(-3), T(-1), T(1), T(3));
	EXPECT_TRUE(ExpectVec4Near(v1plusv2, this->d->v1 + this->d->v2));
	EXPECT_TRUE(ExpectVec4Near(v1plusv2, this->d->v2 + this->d->v1));
	EXPECT_TRUE(ExpectVec4Near(v1minusv2, this->d->v1 - this->d->v2));
}

TYPED_TEST(MathVector4Test, MultiplyDivide)
{
	typedef TypeParam T;
	Math::TVec4<T> v1s2(T(2), T(4), T(6), T(8));
	Math::TVec4<T> v1v2(T(4), T(6), T(6), T(4));
	EXPECT_TRUE(ExpectVec4Near(v1s2, this->d->v1 * T(2)));
	EXPECT_TRUE(ExpectVec4Near(v1s2, T(2) * this->d->v1));
	EXPECT_TRUE(ExpectVec4Near(v1v2, this->d->v1 * this->d->v2));
	EXPECT_TRUE(ExpectVec4Near(this->d->v1, v1s2 / T(2)));
	EXPECT_TRUE(ExpectVec4Near(this->d->v1, v1v2 / this->d->v2));
}

TYPED_TEST(MathVector4Test, Unary)
{
	typedef TypeParam T;
	Math::TVec4<T> expect(T(-1), T(-2), T(-3), T(-4));
	EXPECT_TRUE(ExpectVec4Near(expect, -this->d->v1));
}

TYPED_TEST(MathVector4Test, BinaryOperator)
{
	typedef TypeParam T;
	EXPECT_TRUE(this->d->v1 == this->d->v1);
	EXPECT_TRUE(this->d->v1 != this->d->v2);
}

TYPED_TEST(MathVector4Test, Length)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(3), Math::Length(this->d->v3)));
	EXPECT_TRUE(ExpectNear(T(9), Math::Length2(this->d->v3)));
}

TYPED_TEST(MathVector4Test, Normalize)
{
	typedef TypeParam T;
	Math::TVec4<T> expect = this->d->v3 / T(3);
	EXPECT_TRUE(ExpectVec4Near(expect, Math::Normalize(this->d->v3)));
}

TYPED_TEST(MathVector4Test, Dot)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectNear(T(20), Math::Dot(this->d->v1, this->d->v2)));
}

TYPED_TEST(MathVector4Test, MinMax)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectVec4Near(Math::TVec4<T>(T(1), T(2), T(2), T(1)), Math::Min(this->d->v1, this->d->v2)));
	EXPECT_TRUE(ExpectVec4Near(Math::TVec4<T>(T(4), T(3), T(3), T(4)), Math::Max(this->d->v1, this->d->v2)));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
