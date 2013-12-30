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
struct MathMatrix3Test_Data : public Aligned<std::alignment_of<Math::TMat3<T>>::value>
{

	MathMatrix3Test_Data()
	{
		m1 = Math::TMat3<T>(
			T(1), T(2), T(3),
			T(4), T(5), T(6),
			T(7), T(8), T(9));

		m2 = Math::TMat3<T>(
			T(1), T(4), T(7),
			T(2), T(5), T(8),
			T(3), T(6), T(9));

		m1s2 = Math::TMat3<T>(
			T(2), T(4), T(6),
			T(8), T(10), T(12),
			T(14), T(16), T(18));

		m1m2 = Math::TMat3<T>(
			T(66), T(78), T(90),
			T(78), T(93), T(108),
			T(90), T(108), T(126));

		v1 = Math::TVec3<T>(T(3), T(2), T(1));
		m1v1 = Math::TVec3<T>(T(18), T(24), T(30));
	}

	Math::TMat3<T> zero, identity;
	Math::TMat3<T> m1, m2, m3;
	Math::TMat3<T> m1s2, m1m2;
	Math::TVec3<T> v1, m1v1;

};

template <typename T>
class MathMatrix3Test : public MathTestBase<T>
{
public:

	MathMatrix3Test() { d = new MathMatrix3Test_Data<T>(); }
	virtual ~MathMatrix3Test() { NANON_SAFE_DELETE(d); }

protected:

	MathMatrix3Test_Data<T>* d;

};

TYPED_TEST_CASE(MathMatrix3Test, MathTestTypes);

TYPED_TEST(MathMatrix3Test, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*3+j+1), d->m1.v[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix3Test, Conversion)
{
	typedef TypeParam T;
	Math::TMat4<T> t1(
		T(1), T(2), T(3), T(4),
		T(4), T(5), T(6), T(8),
		T(7), T(8), T(9), T(12),
		T(13), T(14), T(15), T(16));
	EXPECT_TRUE(ExpectMat3Near(d->m1, Math::TMat3<T>(t1)));
}

TYPED_TEST(MathMatrix3Test, Accessor)
{
	typedef TypeParam T;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*3+j+1), d->m1[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix3Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TMat3<T> t;

	t = d->m1; t *= T(2);
	EXPECT_TRUE(ExpectMat3Near(d->m1s2, t));

	t = d->m1; t *= d->m2;
	EXPECT_TRUE(ExpectMat3Near(d->m1m2, t));

	t = d->m1s2; t /= T(2);
	EXPECT_TRUE(ExpectMat3Near(d->m1, t));
}

TYPED_TEST(MathMatrix3Test, MultiplyDivide)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectMat3Near(d->m1s2, d->m1 * T(2)));
	EXPECT_TRUE(ExpectMat3Near(d->m1s2, T(2) * d->m1));
	EXPECT_TRUE(ExpectVec3Near(d->m1v1, d->m1 * d->v1));
	EXPECT_TRUE(ExpectMat3Near(d->m1m2, d->m1 * d->m2));
	EXPECT_TRUE(ExpectMat3Near(d->m1, d->m1s2 / T(2)));
}

// --------------------------------------------------------------------------------


template <typename T>
struct MathMatrix4Test_Data : public Aligned<std::alignment_of<Math::TMat4<T>>::value>
{

	MathMatrix4Test_Data()
	{
		m1 = Math::TMat4<T>(
			T(1), T(2), T(3), T(4),
			T(5), T(6), T(7), T(8),
			T(9), T(10), T(11), T(12),
			T(13), T(14), T(15), T(16));

		m2 = Math::TMat4<T>(
			T(1), T(5), T(9), T(13),
			T(2), T(6), T(10), T(14),
			T(3), T(7), T(11), T(15),
			T(4), T(8), T(12), T(16));

		m1s2 = Math::TMat4<T>(
			T(2), T(4), T(6), T(8),
			T(10), T(12), T(14), T(16),
			T(18), T(20), T(22), T(24),
			T(26), T(28), T(30), T(32));

		m1m2 = Math::TMat4<T>(
			T(276), T(304), T(332), T(360),
			T(304), T(336), T(368), T(400),
			T(332), T(368), T(404), T(440),
			T(360), T(400), T(440), T(480));

		v1 = Math::TVec4<T>(T(4), T(3), T(2), T(1));
		m1v1 = Math::TVec4<T>(T(50), T(60), T(70), T(80));
	}

	Math::TMat4<T> zero, identity;
	Math::TMat4<T> m1, m2, m3;
	Math::TMat4<T> m1s2, m1m2;
	Math::TVec4<T> v1, m1v1;

};

template <typename T>
class MathMatrix4Test : public MathTestBase<T>
{
public:

	MathMatrix4Test() { d = new MathMatrix4Test_Data<T>(); }
	virtual ~MathMatrix4Test() { NANON_SAFE_DELETE(d); }

protected:

	MathMatrix4Test_Data<T>* d;

};

TYPED_TEST_CASE(MathMatrix4Test, MathTestTypes);

TYPED_TEST(MathMatrix4Test, Constructor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), d->m1.v[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix4Test, Conversion)
{
	typedef TypeParam T;
	Math::TMat3<T> t1(
		T(1), T(2), T(3),
		T(4), T(5), T(6),
		T(7), T(8), T(9));
	Math::TMat4<T> expect(
		T(1), T(2), T(3), T(0),
		T(4), T(5), T(6), T(0),
		T(7), T(8), T(9), T(0),
		T(0), T(0), T(0), T(1));
	EXPECT_TRUE(ExpectMat4Near(expect, Math::TMat4<T>(t1)));
}

TYPED_TEST(MathMatrix4Test, Accessor)
{
	typedef TypeParam T;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EXPECT_TRUE(ExpectNear(T(i*4+j+1), d->m1[i][j]));
		}
	}
}

TYPED_TEST(MathMatrix4Test, MultiplyDivideAssign)
{
	typedef TypeParam T;
	Math::TMat4<T> t;

	t = d->m1; t *= T(2);
	EXPECT_TRUE(ExpectMat4Near(d->m1s2, t));

	t = d->m1; t *= d->m2;
	EXPECT_TRUE(ExpectMat4Near(d->m1m2, t));

	t = d->m1s2; t /= T(2);
	EXPECT_TRUE(ExpectMat4Near(d->m1, t));
}

TYPED_TEST(MathMatrix4Test, MultiplyDivide)
{
	typedef TypeParam T;
	EXPECT_TRUE(ExpectMat4Near(d->m1s2, d->m1 * T(2)));
	EXPECT_TRUE(ExpectMat4Near(d->m1s2, T(2) * d->m1));
	EXPECT_TRUE(ExpectVec4Near(d->m1v1, d->m1 * d->v1));
	EXPECT_TRUE(ExpectMat4Near(d->m1m2, d->m1 * d->m2));
	EXPECT_TRUE(ExpectMat4Near(d->m1, d->m1s2 / T(2)));
}

TYPED_TEST(MathMatrix4Test, Transpose)
{
	EXPECT_TRUE(ExpectMat4Near(d->m2, Math::Transpose(d->m1)));
}

TYPED_TEST(MathMatrix4Test, Inverse)
{
	typedef TypeParam T;
	
	// The matrix is orthogonal, so A^-1 should be A^T
	Math::TMat4<T> A(
		T( 0.5), T( 0.5), T( 0.5), T(-0.5),
		T(-0.5), T( 0.5), T( 0.5), T( 0.5),
		T( 0.5), T(-0.5), T( 0.5), T( 0.5),
		T( 0.5), T( 0.5), T(-0.5), T( 0.5));

	Math::TMat4<T> AT(
		T( 0.5), T(-0.5), T( 0.5), T( 0.5),
		T( 0.5), T( 0.5), T(-0.5), T( 0.5),
		T( 0.5), T( 0.5), T( 0.5), T(-0.5),
		T(-0.5), T( 0.5), T( 0.5), T( 0.5));

	EXPECT_TRUE(ExpectMat4Near(AT, Math::Inverse(A)));
}

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END