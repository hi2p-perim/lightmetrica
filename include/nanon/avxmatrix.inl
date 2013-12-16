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

NANON_FORCE_INLINE Mat4d::TMat4()
{

}

NANON_FORCE_INLINE Mat4d::TMat4(const Mat4d& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
	v[3] = m.v[3];
}

NANON_FORCE_INLINE Mat4d::TMat4(double v)
{
	this->v[0] = Vec4d(v);
	this->v[1] = Vec4d(v);
	this->v[2] = Vec4d(v);
	this->v[3] = Vec4d(v);
}

NANON_FORCE_INLINE Mat4d::TMat4(const Vec4d& v0, const Vec4d& v1, const Vec4d& v2, const Vec4d& v3)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
}

NANON_FORCE_INLINE Mat4d::TMat4(const double* v)
{
	this->v[0] = Vec4d(v[0], v[1], v[2], v[3]);
	this->v[1] = Vec4d(v[4], v[5], v[6], v[7]);
	this->v[2] = Vec4d(v[8], v[9], v[10], v[11]);
	this->v[3] = Vec4d(v[12], v[13], v[14], v[15]);
}

NANON_FORCE_INLINE Mat4d::TMat4(
	double v00, double v10, double v20, double v30,
	double v01, double v11, double v21, double v31,
	double v02, double v12, double v22, double v32,
	double v03, double v13, double v23, double v33)
{
	v[0] = Vec4d(v00, v10, v20, v30);
	v[1] = Vec4d(v01, v11, v21, v31);
	v[2] = Vec4d(v02, v12, v22, v32);
	v[3] = Vec4d(v03, v13, v23, v33);
}

NANON_FORCE_INLINE Mat4d Mat4d::Zero()
{
	return TMat4<double>();
}

NANON_FORCE_INLINE Mat4d Mat4d::Diag(double v)
{
	return TMat4<double>(
		v, 0, 0, 0,
		0, v, 0, 0,
		0, 0, v, 0,
		0, 0, 0, v);
}

NANON_FORCE_INLINE Mat4d Mat4d::Identity()
{
	return Diag(1);
}

NANON_FORCE_INLINE Vec4d& Mat4d::operator[](int i)
{
	return v[i];
}

NANON_FORCE_INLINE const Vec4d& Mat4d::operator[](int i) const
{
	return v[i];
}

NANON_FORCE_INLINE Mat4d operator*(const Mat4d& m, double s)
{
	return Mat4d(m[0] * s, m[1] * s, m[2] * s, m[3] * s);
}

NANON_FORCE_INLINE Mat4d operator*(double s, const Mat4d& m)
{
	return m * s;
}

NANON_FORCE_INLINE Vec4d operator*(const Mat4d& m, const Vec4d& v)
{
	return Vec4d(
		_mm256_add_pd(
			_mm256_add_pd(
				_mm256_mul_pd(m[0].v, _mm256_broadcast_sd(&(v.x))),
				_mm256_mul_pd(m[1].v, _mm256_broadcast_sd(&(v.x) + 1))),
			_mm256_add_pd(
				_mm256_mul_pd(m[2].v, _mm256_broadcast_sd(&(v.x) + 2)),
				_mm256_mul_pd(m[3].v, _mm256_broadcast_sd(&(v.x) + 3)))));
}

NANON_FORCE_INLINE Mat4d operator*(const Mat4d& m1, const Mat4d& m2)
{
	return Mat4d(m1 * m2[0], m1 * m2[1], m1 * m2[2], m1 * m2[3]);
}

NANON_NAMESPACE_END