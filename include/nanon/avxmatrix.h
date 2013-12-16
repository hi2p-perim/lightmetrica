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

#ifndef __LIB_NANON_AVX_MATRIX_H__
#define __LIB_NANON_AVX_MATRIX_H__

#include "matrix.h"
#include "avxvector.h"

NANON_NAMESPACE_BEGIN

/*!
	AVX optimized 3x3 matrix.
	Specialized version of TMat3 optimized by AVX.
*/
template <>
struct NANON_ALIGN_16 TMat3<double>
{
	
	Vec3d v[3];

	NANON_FORCE_INLINE TMat3();
	NANON_FORCE_INLINE TMat3(const Mat3d& m);
	NANON_FORCE_INLINE TMat3(double v);
	NANON_FORCE_INLINE TMat3(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2);
	NANON_FORCE_INLINE TMat3(const double* v);
	NANON_FORCE_INLINE TMat3(
		double v00, double v10, double v20,
		double v01, double v11, double v21,
		double v02, double v12, double v22);

	static NANON_FORCE_INLINE Mat3d Zero();
	static NANON_FORCE_INLINE Mat3d Diag(double v);
	static NANON_FORCE_INLINE Mat3d Identity();

	NANON_FORCE_INLINE Vec3d& operator[](int i);
	NANON_FORCE_INLINE const Vec3d& operator[](int i) const;

};

// --------------------------------------------------------------------------------

/*!
	AVX optimized 4x4 matrix.
	Specialized version of TMat4 optimized by AVX.
*/
template <>
struct NANON_ALIGN_16 TMat4<double>
{
	
	Vec4d v[4];

	NANON_FORCE_INLINE TMat4();
	NANON_FORCE_INLINE TMat4(const Mat4d& m);
	NANON_FORCE_INLINE TMat4(double v);
	NANON_FORCE_INLINE TMat4(const Vec4d& v0, const Vec4d& v1, const Vec4d& v2, const Vec4d& v3);
	NANON_FORCE_INLINE TMat4(const double* v);
	NANON_FORCE_INLINE TMat4(
		double v00, double v10, double v20, double v30,
		double v01, double v11, double v21, double v31,
		double v02, double v12, double v22, double v32,
		double v03, double v13, double v23, double v33);

	static NANON_FORCE_INLINE Mat4d Zero();
	static NANON_FORCE_INLINE Mat4d Diag(double v);
	static NANON_FORCE_INLINE Mat4d Identity();

	NANON_FORCE_INLINE Vec4d& operator[](int i);
	NANON_FORCE_INLINE const Vec4d& operator[](int i) const;

};

NANON_NAMESPACE_END

#include "avxmatrix.inl"

#endif //__LIB_NANON_AVX_MATRIX_H__