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

#pragma once
#ifndef __LIB_LIGHTMETRICA_MATH_MATRIX_H__
#define __LIB_LIGHTMETRICA_MATH_MATRIX_H__

#include "math.vector.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> struct TMat3;
template <typename T> struct TMat4;

/*!
	3x3 matrix.
	Generic column major 3x3 matrix. 
	A matrix
		v00 v01 v02
		v10 v11 v12
		v20 v21 v22
	is stored sequentially as v00, v10, ..., v22.
	\tparam T Internal value type.
*/
template <typename T>
struct TMat3
{

	TVec3<T> v[3];

	LM_FORCE_INLINE TMat3();
	LM_FORCE_INLINE TMat3(const TMat3<T>& m);
	LM_FORCE_INLINE TMat3(const TMat4<T>& m);
	LM_FORCE_INLINE TMat3(const T& v);
	LM_FORCE_INLINE TMat3(const TVec3<T>& v0, const TVec3<T>& v1, const TVec3<T>& v2);
	LM_FORCE_INLINE TMat3(const T* v);
	LM_FORCE_INLINE TMat3(
		T v00, T v10, T v20,
		T v01, T v11, T v21,
		T v02, T v12, T v22);

	static LM_FORCE_INLINE TMat3<T> Zero();
	static LM_FORCE_INLINE TMat3<T> Diag(T v);
	static LM_FORCE_INLINE TMat3<T> Identity();

	LM_FORCE_INLINE TVec3<T>& operator[](int i);
	LM_FORCE_INLINE const TVec3<T>& operator[](int i) const;

	LM_FORCE_INLINE TMat3<T>& operator*=(const TMat3<T>& m);
	LM_FORCE_INLINE TMat3<T>& operator*=(const T& s);
	LM_FORCE_INLINE TMat3<T>& operator/=(const T& s);

};

template <typename T> LM_FORCE_INLINE TMat3<T> operator*(const TMat3<T>& m, const T& s);
template <typename T> LM_FORCE_INLINE TMat3<T> operator*(const T& v, const TMat3<T>& m);
template <typename T> LM_FORCE_INLINE TVec3<T> operator*(const TMat3<T>& m, const TVec3<T>& v);
template <typename T> LM_FORCE_INLINE TMat3<T> operator*(const TMat3<T>& m1, const TMat3<T>& m2);
template <typename T> LM_FORCE_INLINE TMat3<T> operator/(const TMat3<T>& m, const T& s);

template <typename T> LM_FORCE_INLINE TMat3<T> Transpose(const TMat3<T>& m);
template <typename T> LM_FORCE_INLINE TMat3<T> Inverse(const TMat3<T>& m);

typedef TMat3<float> Mat3f;
typedef TMat3<double> Mat3d;
typedef TMat3<int> Mat3i;

// --------------------------------------------------------------------------------

/*!
	4x4 matrix.
	Generic column major 4x4 matrix. 
	A matrix
		v00 v01 v02 v03
		v10 v11 v12 v13
		v20 v21 v22 v23
		v30 v31 v32 v33
	is stored sequentially as v00, v10, ..., v33.
	\tparam T Internal value type.
*/
template <typename T>
struct TMat4
{

	TVec4<T> v[4];

	LM_FORCE_INLINE TMat4();
	LM_FORCE_INLINE TMat4(const TMat3<T>& m);
	LM_FORCE_INLINE TMat4(const TMat4<T>& m);
	LM_FORCE_INLINE TMat4(const T& v);
	LM_FORCE_INLINE TMat4(const TVec4<T>& v0, const TVec4<T>& v1, const TVec4<T>& v2, const TVec4<T>& v3);
	LM_FORCE_INLINE TMat4(const T* v);
	LM_FORCE_INLINE TMat4(
		T v00, T v10, T v20, T v30,
		T v01, T v11, T v21, T v31,
		T v02, T v12, T v22, T v32,
		T v03, T v13, T v23, T v33);

	static LM_FORCE_INLINE TMat4<T> Zero();
	static LM_FORCE_INLINE TMat4<T> Diag(T v);
	static LM_FORCE_INLINE TMat4<T> Identity();

	LM_FORCE_INLINE TVec4<T>& operator[](int i);
	LM_FORCE_INLINE const TVec4<T>& operator[](int i) const;

	LM_FORCE_INLINE TMat4<T>& operator*=(const TMat4<T>& m);
	LM_FORCE_INLINE TMat4<T>& operator*=(const T& s);
	LM_FORCE_INLINE TMat4<T>& operator/=(const T& s);

};

template <typename T> LM_FORCE_INLINE TMat4<T> operator*(const TMat4<T>& m, const T& s);
template <typename T> LM_FORCE_INLINE TMat4<T> operator*(const T& v, const TMat4<T>& m);
template <typename T> LM_FORCE_INLINE TVec4<T> operator*(const TMat4<T>& m, const TVec4<T>& v);
template <typename T> LM_FORCE_INLINE TMat4<T> operator*(const TMat4<T>& m1, const TMat4<T>& m2);
template <typename T> LM_FORCE_INLINE TMat4<T> operator/(const TMat4<T>& m, const T& s);

template <typename T> LM_FORCE_INLINE TMat4<T> Transpose(const TMat4<T>& m);
template <typename T> LM_FORCE_INLINE TMat4<T> Inverse(const TMat4<T>& m);

typedef TMat4<float> Mat4f;
typedef TMat4<double> Mat4d;
typedef TMat4<int> Mat4i;

// --------------------------------------------------------------------------------

#ifdef LM_USE_SSE2

template <> struct LM_ALIGN_16 TMat3<float>;
template <> struct LM_ALIGN_16 TMat4<float>;

/*!
	SSE optimized 3x3 matrix.
	Specialized version of TMat3 optimized by SSE.
*/
template <>
struct LM_ALIGN_16 TMat3<float>
{
	
	Vec3f v[3];

	LM_FORCE_INLINE TMat3();
	LM_FORCE_INLINE TMat3(const Mat3f& m);
	LM_FORCE_INLINE TMat3(const Mat4f& m);
	LM_FORCE_INLINE TMat3(float v);
	LM_FORCE_INLINE TMat3(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2);
	LM_FORCE_INLINE TMat3(const float* v);
	LM_FORCE_INLINE TMat3(
		float v00, float v10, float v20,
		float v01, float v11, float v21,
		float v02, float v12, float v22);

	static LM_FORCE_INLINE Mat3f Zero();
	static LM_FORCE_INLINE Mat3f Diag(float v);
	static LM_FORCE_INLINE Mat3f Identity();

	LM_FORCE_INLINE Vec3f& operator[](int i);
	LM_FORCE_INLINE const Vec3f& operator[](int i) const;

	LM_FORCE_INLINE Mat3f& operator*=(const Mat3f& m);
	LM_FORCE_INLINE Mat3f& operator*=(const float& s);
	LM_FORCE_INLINE Mat3f& operator/=(const float& s);

};

template <> LM_FORCE_INLINE Mat3f operator*(const Mat3f& m, const float& s);
template <> LM_FORCE_INLINE Mat3f operator*(const float& s, const Mat3f& m);
template <> LM_FORCE_INLINE Vec3f operator*(const Mat3f& m, const Vec3f& v);
template <> LM_FORCE_INLINE Mat3f operator*(const Mat3f& m1, const Mat3f& m2);
template <> LM_FORCE_INLINE Mat3f operator/(const Mat3f& m, const float& s);

template <> LM_FORCE_INLINE Mat3f Transpose(const Mat3f& m);
//template <> LM_FORCE_INLINE Mat4f Inverse(const Mat4f& m);

// --------------------------------------------------------------------------------

/*!
	SSE optimized 4x4 matrix.
	Specialized version of TMat4 optimized by SSE.
*/
template <>
struct LM_ALIGN_16 TMat4<float>
{
	
	Vec4f v[4];

	LM_FORCE_INLINE TMat4();
	LM_FORCE_INLINE TMat4(const Mat3f& m);
	LM_FORCE_INLINE TMat4(const Mat4f& m);
	LM_FORCE_INLINE TMat4(float v);
	LM_FORCE_INLINE TMat4(const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, const Vec4f& v3);
	LM_FORCE_INLINE TMat4(const float* v);
	LM_FORCE_INLINE TMat4(
		float v00, float v10, float v20, float v30,
		float v01, float v11, float v21, float v31,
		float v02, float v12, float v22, float v32,
		float v03, float v13, float v23, float v33);

	static LM_FORCE_INLINE Mat4f Zero();
	static LM_FORCE_INLINE Mat4f Diag(float v);
	static LM_FORCE_INLINE Mat4f Identity();

	LM_FORCE_INLINE Vec4f& operator[](int i);
	LM_FORCE_INLINE const Vec4f& operator[](int i) const;

	LM_FORCE_INLINE Mat4f& operator*=(const Mat4f& m);
	LM_FORCE_INLINE Mat4f& operator*=(const float& s);
	LM_FORCE_INLINE Mat4f& operator/=(const float& s);

};

template <> LM_FORCE_INLINE Mat4f operator*(const Mat4f& m, const float& s);
template <> LM_FORCE_INLINE Mat4f operator*(const float& s, const Mat4f& m);
template <> LM_FORCE_INLINE Vec4f operator*(const Mat4f& m, const Vec4f& v);
template <> LM_FORCE_INLINE Mat4f operator*(const Mat4f& m1, const Mat4f& m2);
template <> LM_FORCE_INLINE Mat4f operator/(const Mat4f& m, const float& s);

template <> LM_FORCE_INLINE Mat4f Transpose(const Mat4f& m);
template <> LM_FORCE_INLINE Mat4f Inverse(const Mat4f& m);

#endif

// --------------------------------------------------------------------------------

#ifdef LM_USE_AVX

template <> struct LM_ALIGN_32 TMat3<double>;
template <> struct LM_ALIGN_32 TMat4<double>;

/*!
	AVX optimized 3x3 matrix.
	Specialized version of TMat3 optimized by AVX.
*/
template <>
struct LM_ALIGN_32 TMat3<double>
{
	
	Vec3d v[3];

	LM_FORCE_INLINE TMat3();
	LM_FORCE_INLINE TMat3(const Mat3d& m);
	LM_FORCE_INLINE TMat3(const Mat4d& m);
	LM_FORCE_INLINE TMat3(double v);
	LM_FORCE_INLINE TMat3(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2);
	LM_FORCE_INLINE TMat3(const double* v);
	LM_FORCE_INLINE TMat3(
		double v00, double v10, double v20,
		double v01, double v11, double v21,
		double v02, double v12, double v22);

	static LM_FORCE_INLINE Mat3d Zero();
	static LM_FORCE_INLINE Mat3d Diag(double v);
	static LM_FORCE_INLINE Mat3d Identity();

	LM_FORCE_INLINE Vec3d& operator[](int i);
	LM_FORCE_INLINE const Vec3d& operator[](int i) const;

	LM_FORCE_INLINE Mat3d& operator*=(const Mat3d& m);
	LM_FORCE_INLINE Mat3d& operator*=(const double& s);
	LM_FORCE_INLINE Mat3d& operator/=(const double& s);

};

template <> LM_FORCE_INLINE Mat3d operator*(const Mat3d& m, const double& s);
template <> LM_FORCE_INLINE Mat3d operator*(const double& s, const Mat3d& m);
template <> LM_FORCE_INLINE Vec3d operator*(const Mat3d& m, const Vec3d& v);
template <> LM_FORCE_INLINE Mat3d operator*(const Mat3d& m1, const Mat3d& m2);
template <> LM_FORCE_INLINE Mat3d operator/(const Mat3d& m, const double& s);

// --------------------------------------------------------------------------------

/*!
	AVX optimized 4x4 matrix.
	Specialized version of TMat4 optimized by AVX.
*/
template <>
struct LM_ALIGN_32 TMat4<double>
{
	
	Vec4d v[4];

	LM_FORCE_INLINE TMat4();
	LM_FORCE_INLINE TMat4(const Mat3d& m);
	LM_FORCE_INLINE TMat4(const Mat4d& m);
	LM_FORCE_INLINE TMat4(double v);
	LM_FORCE_INLINE TMat4(const Vec4d& v0, const Vec4d& v1, const Vec4d& v2, const Vec4d& v3);
	LM_FORCE_INLINE TMat4(const double* v);
	LM_FORCE_INLINE TMat4(
		double v00, double v10, double v20, double v30,
		double v01, double v11, double v21, double v31,
		double v02, double v12, double v22, double v32,
		double v03, double v13, double v23, double v33);

	static LM_FORCE_INLINE Mat4d Zero();
	static LM_FORCE_INLINE Mat4d Diag(double v);
	static LM_FORCE_INLINE Mat4d Identity();

	LM_FORCE_INLINE Vec4d& operator[](int i);
	LM_FORCE_INLINE const Vec4d& operator[](int i) const;

	LM_FORCE_INLINE Mat4d& operator*=(const Mat4d& m);
	LM_FORCE_INLINE Mat4d& operator*=(const double& s);
	LM_FORCE_INLINE Mat4d& operator/=(const double& s);

};

template <> LM_FORCE_INLINE Mat4d operator*(const Mat4d& m, const double& s);
template <> LM_FORCE_INLINE Mat4d operator*(const double& s, const Mat4d& m);
template <> LM_FORCE_INLINE Vec4d operator*(const Mat4d& m, const Vec4d& v);
template <> LM_FORCE_INLINE Mat4d operator*(const Mat4d& m1, const Mat4d& m2);
template <> LM_FORCE_INLINE Mat4d operator/(const Mat4d& m, const double& s);

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.matrix.inl"

#endif // __LIB_LIGHTMETRICA_MATH_MATRIX_H__