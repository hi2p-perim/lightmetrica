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

#ifndef __LIB_NANON_MATRIX_H__
#define __LIB_NANON_MATRIX_H__

#include "common.h"
#include "vector.h"

NANON_NAMESPACE_BEGIN

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

	NANON_FORCE_INLINE TMat4();
	NANON_FORCE_INLINE TMat4(const TMat4<T>& m);
	NANON_FORCE_INLINE TMat4(const T& v);
	NANON_FORCE_INLINE TMat4(const TVec4<T>& v0, const TVec4<T>& v1, const TVec4<T>& v2, const TVec4<T>& v3);
	NANON_FORCE_INLINE TMat4(
		T v00, T v10, T v20, T v30,
		T v01, T v11, T v21, T v31,
		T v02, T v12, T v22, T v32,
		T v03, T v13, T v23, T v33);

	static NANON_FORCE_INLINE TMat4<T> Zero();
	static NANON_FORCE_INLINE TMat4<T> Diag(T v);
	static NANON_FORCE_INLINE TMat4<T> Identity();

	NANON_FORCE_INLINE TVec4<T>& operator[](int i);
	NANON_FORCE_INLINE const TVec4<T>& operator[](int i) const;

};

typedef TMat4<float> Mat4f;
typedef TMat4<double> Mat4d;
typedef TMat4<int> Mat4i;

NANON_NAMESPACE_END

#include "matrix.inl"

#endif // __LIB_NANON_MATRIX_H__