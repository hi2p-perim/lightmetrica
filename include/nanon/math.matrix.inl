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

#include "math.matrix.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3()
{

}

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3(const TMat3<T>& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
}

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3(const T& v)
{
	this->v[0] = TVec3<T>(v);
	this->v[1] = TVec3<T>(v);
	this->v[2] = TVec3<T>(v);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3(const TVec3<T>& v0, const TVec3<T>& v1, const TVec3<T>& v2)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
}

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3(const T* v)
{
	this->v[0] = TVec3<T>(v[0], v[1], v[2]);
	this->v[1] = TVec3<T>(v[3], v[4], v[5]);
	this->v[2] = TVec3<T>(v[6], v[7], v[8]);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T>::TMat3(
	T v00, T v10, T v20,
	T v01, T v11, T v21,
	T v02, T v12, T v22)
{
	v[0] = TVec3<T>(v00, v10, v20);
	v[1] = TVec3<T>(v01, v11, v21);
	v[2] = TVec3<T>(v02, v12, v22);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> TMat3<T>::Zero()
{
	return TMat3<T>();
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> TMat3<T>::Diag(T v)
{
	T Zero(0);
	return TMat3<T>(
		v, Zero, Zero,
		Zero, v, Zero,
		Zero, Zero, v);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> TMat3<T>::Identity()
{
	return Diag(T(1));
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TMat3<T>::operator[](int i)
{
	return v[i];
}

template <typename T>
NANON_FORCE_INLINE const TVec3<T>& TMat3<T>::operator[](int i) const
{
	return v[i];
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> operator*(const TMat3<T>& m, const T& s)
{
	return TMat3<T>(m[0] * s, m[1] * s, m[2] * s);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> operator*(const T& v, const TMat3<T>& m)
{
	return m * v;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator*(const TMat3<T>& m, const TVec3<T>& v)
{
	return TVec3<T>(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z);
}

template <typename T>
NANON_FORCE_INLINE TMat3<T> operator*(const TMat3<T>& m1, const TMat3<T>& m2)
{
	return TMat3<T>(m1 * m2[0], m1 * m2[1], m1 * m2[2]);
}

// --------------------------------------------------------------------------------

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4()
{

}

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4(const TMat4<T>& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
	v[3] = m.v[3];
}

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4(const T& v)
{
	this->v[0] = TVec4<T>(v);
	this->v[1] = TVec4<T>(v);
	this->v[2] = TVec4<T>(v);
	this->v[3] = TVec4<T>(v);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4(const TVec4<T>& v0, const TVec4<T>& v1, const TVec4<T>& v2, const TVec4<T>& v3)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
	v[3] = v3;
}

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4(const T* v)
{
	this->v[0] = TVec4<T>(v[0], v[1], v[2], v[3]);
	this->v[1] = TVec4<T>(v[4], v[5], v[6], v[7]);
	this->v[2] = TVec4<T>(v[8], v[9], v[10], v[11]);
	this->v[3] = TVec4<T>(v[12], v[13], v[14], v[15]);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T>::TMat4(
	T v00, T v10, T v20, T v30,
	T v01, T v11, T v21, T v31,
	T v02, T v12, T v22, T v32,
	T v03, T v13, T v23, T v33)
{
	v[0] = TVec4<T>(v00, v10, v20, v30);
	v[1] = TVec4<T>(v01, v11, v21, v31);
	v[2] = TVec4<T>(v02, v12, v22, v32);
	v[3] = TVec4<T>(v03, v13, v23, v33);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> TMat4<T>::Zero()
{
	return TMat4<T>();
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> TMat4<T>::Diag(T v)
{
	T Zero(0);
	return TMat4<T>(
		v, Zero, Zero, Zero,
		Zero, v, Zero, Zero,
		Zero, Zero, v, Zero,
		Zero, Zero, Zero, v);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> TMat4<T>::Identity()
{
	return Diag(T(1));
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TMat4<T>::operator[](int i)
{
	return v[i];
}

template <typename T>
NANON_FORCE_INLINE const TVec4<T>& TMat4<T>::operator[](int i) const
{
	return v[i];
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> operator*(const TMat4<T>& m, const T& s)
{
	return TMat4<T>(m[0] * s, m[1] * s, m[2] * s, m[3] * s);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> operator*(const T& v, const TMat4<T>& m)
{
	return m * v;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator*(const TMat4<T>& m, const TVec4<T>& v)
{
	return TVec4<T>(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
		m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w);
}

template <typename T>
NANON_FORCE_INLINE TMat4<T> operator*(const TMat4<T>& m1, const TMat4<T>& m2)
{
	return TMat4<T>(m1 * m2[0], m1 * m2[1], m1 * m2[2], m1 * m2[3]);
}

// --------------------------------------------------------------------------------

#ifdef NANON_USE_SSE2

NANON_FORCE_INLINE Mat3f::TMat3()
{

}

NANON_FORCE_INLINE Mat3f::TMat3(const Mat3f& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
}

NANON_FORCE_INLINE Mat3f::TMat3(float v)
{
	this->v[0] = Vec3f(v);
	this->v[1] = Vec3f(v);
	this->v[2] = Vec3f(v);
}

NANON_FORCE_INLINE Mat3f::TMat3(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
}

NANON_FORCE_INLINE Mat3f::TMat3(const float* v)
{
	this->v[0] = Vec3f(v[0], v[1], v[2]);
	this->v[1] = Vec3f(v[3], v[4], v[5]);
	this->v[2] = Vec3f(v[6], v[7], v[8]);
}

NANON_FORCE_INLINE Mat3f::TMat3(
	float v00, float v10, float v20,
	float v01, float v11, float v21,
	float v02, float v12, float v22)
{
	v[0] = Vec3f(v00, v10, v20);
	v[1] = Vec3f(v01, v11, v21);
	v[2] = Vec3f(v02, v12, v22);
}

NANON_FORCE_INLINE Mat3f Mat3f::Zero()
{
	return TMat3<float>();
}

NANON_FORCE_INLINE Mat3f Mat3f::Diag(float v)
{
	return TMat3<float>(
		v, 0.0f, 0.0f,
		0.0f, v, 0.0f,
		0.0f, 0.0f, v);
}

NANON_FORCE_INLINE Mat3f Mat3f::Identity()
{
	return Diag(1.0f);
}

NANON_FORCE_INLINE Vec3f& Mat3f::operator[](int i)
{
	return v[i];
}

NANON_FORCE_INLINE const Vec3f& Mat3f::operator[](int i) const
{
	return v[i];
}

NANON_FORCE_INLINE Mat3f operator*(const Mat3f& m, float s)
{
	return Mat3f(m[0] * s, m[1] * s, m[2] * s);
}

NANON_FORCE_INLINE Mat3f operator*(float s, const Mat3f& m)
{
	return m * s;
}

NANON_FORCE_INLINE Vec3f operator*(const Mat3f& m, const Vec3f& v)
{
	return Vec3f(
		_mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(m[0].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(0, 0, 0, 0))),
				_mm_mul_ps(m[1].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(1, 1, 1, 1)))),
			_mm_add_ps(
				_mm_mul_ps(m[2].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(2, 2, 2, 2))),
				_mm_mul_ps(m[3].v, _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(3, 3, 3, 3))))));
}

NANON_FORCE_INLINE Mat3f operator*(const Mat3f& m1, const Mat3f& m2)
{
	return Mat3f(m1 * m2[0], m1 * m2[1], m1 * m2[2]);
}

// --------------------------------------------------------------------------------

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

NANON_FORCE_INLINE Mat4f::TMat4(const float* v)
{
	this->v[0] = Vec4f(v[0], v[1], v[2], v[3]);
	this->v[1] = Vec4f(v[4], v[5], v[6], v[7]);
	this->v[2] = Vec4f(v[8], v[9], v[10], v[11]);
	this->v[3] = Vec4f(v[12], v[13], v[14], v[15]);
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

#endif

// --------------------------------------------------------------------------------

#ifdef NANON_USE_AVX

NANON_FORCE_INLINE Mat3d::TMat3()
{

}

NANON_FORCE_INLINE Mat3d::TMat3(const Mat3d& m)
{
	v[0] = m.v[0];
	v[1] = m.v[1];
	v[2] = m.v[2];
}

NANON_FORCE_INLINE Mat3d::TMat3(double v)
{
	this->v[0] = Vec3d(v);
	this->v[1] = Vec3d(v);
	this->v[2] = Vec3d(v);
}

NANON_FORCE_INLINE Mat3d::TMat3(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
}

NANON_FORCE_INLINE Mat3d::TMat3(const double* v)
{
	this->v[0] = Vec3d(v[0], v[1], v[2]);
	this->v[1] = Vec3d(v[4], v[5], v[6]);
	this->v[2] = Vec3d(v[8], v[9], v[10]);
}

NANON_FORCE_INLINE Mat3d::TMat3(
	double v00, double v10, double v20,
	double v01, double v11, double v21,
	double v02, double v12, double v22)
{
	v[0] = Vec3d(v00, v10, v20);
	v[1] = Vec3d(v01, v11, v21);
	v[2] = Vec3d(v02, v12, v22);
}

NANON_FORCE_INLINE Mat3d Mat3d::Zero()
{
	return TMat3<double>();
}

NANON_FORCE_INLINE Mat3d Mat3d::Diag(double v)
{
	return TMat3<double>(
		v, 0, 0,
		0, v, 0,
		0, 0, v);
}

NANON_FORCE_INLINE Mat3d Mat3d::Identity()
{
	return Diag(1);
}

NANON_FORCE_INLINE Vec3d& Mat3d::operator[](int i)
{
	return v[i];
}

NANON_FORCE_INLINE const Vec3d& Mat3d::operator[](int i) const
{
	return v[i];
}

NANON_FORCE_INLINE Mat3d operator*(const Mat3d& m, double s)
{
	return Mat3d(m[0] * s, m[1] * s, m[2] * s);
}

NANON_FORCE_INLINE Mat3d operator*(double s, const Mat3d& m)
{
	return m * s;
}

NANON_FORCE_INLINE Vec3d operator*(const Mat3d& m, const Vec3d& v)
{
	return Vec3d(
		_mm256_add_pd(
			_mm256_add_pd(
				_mm256_mul_pd(m[0].v, _mm256_broadcast_sd(&(v.x))),
				_mm256_mul_pd(m[1].v, _mm256_broadcast_sd(&(v.x) + 1))),
			_mm256_add_pd(
				_mm256_mul_pd(m[2].v, _mm256_broadcast_sd(&(v.x) + 2)),
				_mm256_mul_pd(m[3].v, _mm256_broadcast_sd(&(v.x) + 3)))));
}

NANON_FORCE_INLINE Mat3d operator*(const Mat3d& m1, const Mat3d& m2)
{
	return Mat3d(m1 * m2[0], m1 * m2[1], m1 * m2[2]);
}

// --------------------------------------------------------------------------------

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

#endif

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END