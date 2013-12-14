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

#include "ssematrix.h"

NANON_NAMESPACE_BEGIN

NANON_FORCE_INLINE Mat4f::TMat4()
{

}

NANON_FORCE_INLINE Mat4f::TMat4(const Mat4f& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
	v[3] = m.v[3];
}

NANON_FORCE_INLINE Mat4f::TMat4(float v)
{
	this->v[0] = Vec4f(v);
	this->v[1] = Vec4f(v);
	this->v[2] = Vec4f(v);
	this->v[3] = Vec4f(v);
}

NANON_FORCE_INLINE Mat4f::TMat4(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
}

NANON_FORCE_INLINE Mat4f::TMat4(
	float v00, float v10, float v20, float v30,
	float v01, float v11, float v21, float v31,
	float v02, float v12, float v22, float v32,
	float v03, float v13, float v23, float v33)
{
	v[0] = Vec4f(v00, v10, v20, v30);
	v[1] = Vec4f(v01, v11, v21, v31);
	v[2] = Vec4f(v02, v12, v22, v32);
	v[3] = Vec4f(v03, v13, v23, v33);
}

NANON_FORCE_INLINE Mat4f Mat4f::Zero()
{
	return TMat4<float>();
}

NANON_FORCE_INLINE Mat4f Mat4f::Diag(float v)
{
	return TMat4<float>(
		v, 0.0f, 0.0f, 0.0f,
		0.0f, v, 0.0f, 0.0f,
		0.0f, 0.0f, v, 0.0f,
		0.0f, 0.0f, 0.0f, v);
}

NANON_FORCE_INLINE Mat4f Mat4f::Identity()
{
	return Diag(1.0f);
}

NANON_FORCE_INLINE Vec4f& Mat4f::operator[](int i)
{
	return v[i];
}

NANON_FORCE_INLINE const Vec4f& Mat4f::operator[](int i) const
{
	return v[i];
}

NANON_FORCE_INLINE Mat4f operator*(const Mat4f& m, float s)
{
	return Mat4f(m[0] * s, m[1] * s, m[2] * s, m[3] * s);
}

NANON_FORCE_INLINE Mat4f operator*(float s, const Mat4f& m)
{
	return m * s;
}

NANON_FORCE_INLINE Vec4f operator*(const Mat4f& m, const Vec4f& v)
{
	return Vec4f(
		_mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(m[0].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(0, 0, 0, 0))),
				_mm_mul_ps(m[1].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(1, 1, 1, 1)))),
			_mm_add_ps(
				_mm_mul_ps(m[2].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(2, 2, 2, 2))),
				_mm_mul_ps(m[3].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(3, 3, 3, 3))))));
}

NANON_FORCE_INLINE Mat4f operator*(const Mat4f& m1, const Mat4f& m2)
{
	return Mat4f(m1 * m2[0], m1 * m2[1], m1 * m2[2], m1 * m2[3]);
}

NANON_NAMESPACE_END