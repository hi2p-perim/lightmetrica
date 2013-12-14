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

#include "matrix.h"

NANON_NAMESPACE_BEGIN

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
	//return TMat4<T>(
	//	m1[0][0] * m2[0][0] + m1[1][0] * m2[0][1] + m1[2][0] * m2[0][2] + m1[3][0] * m2[0][3],
	//	m1[0][1] * m2[0][0] + m1[1][1] * m2[0][1] + m1[2][1] * m2[0][2] + m1[3][1] * m2[0][3],
	//	m1[0][2] * m2[0][0] + m1[1][2] * m2[0][1] + m1[2][2] * m2[0][2] + m1[3][2] * m2[0][3],
	//	m1[0][3] * m2[0][0] + m1[1][3] * m2[0][1] + m1[2][3] * m2[0][2] + m1[3][3] * m2[0][3],
	//	m1[0][0] * m2[1][0] + m1[1][0] * m2[1][1] + m1[2][0] * m2[1][2] + m1[3][0] * m2[1][3],
	//	m1[0][1] * m2[1][0] + m1[1][1] * m2[1][1] + m1[2][1] * m2[1][2] + m1[3][1] * m2[1][3],
	//	m1[0][2] * m2[1][0] + m1[1][2] * m2[1][1] + m1[2][2] * m2[1][2] + m1[3][2] * m2[1][3],
	//	m1[0][3] * m2[1][0] + m1[1][3] * m2[1][1] + m1[2][3] * m2[1][2] + m1[3][3] * m2[1][3],
	//	m1[0][0] * m2[2][0] + m1[1][0] * m2[2][1] + m1[2][0] * m2[2][2] + m1[3][0] * m2[2][3],
	//	m1[0][1] * m2[2][0] + m1[1][1] * m2[2][1] + m1[2][1] * m2[2][2] + m1[3][1] * m2[2][3],
	//	m1[0][2] * m2[2][0] + m1[1][2] * m2[2][1] + m1[2][2] * m2[2][2] + m1[3][2] * m2[2][3],
	//	m1[0][3] * m2[2][0] + m1[1][3] * m2[2][1] + m1[2][3] * m2[2][2] + m1[3][3] * m2[2][3],
	//	m1[0][0] * m2[3][0] + m1[1][0] * m2[3][1] + m1[2][0] * m2[3][2] + m1[3][0] * m2[3][3],
	//	m1[0][1] * m2[3][0] + m1[1][1] * m2[3][1] + m1[2][1] * m2[3][2] + m1[3][1] * m2[3][3],
	//	m1[0][2] * m2[3][0] + m1[1][2] * m2[3][1] + m1[2][2] * m2[3][2] + m1[3][2] * m2[3][3],
	//	m1[0][3] * m2[3][0] + m1[1][3] * m2[3][1] + m1[2][3] * m2[3][2] + m1[3][3] * m2[3][3]);
	return TMat4<T>(m1 * m2[0], m1 * m2[1], m1 * m2[2], m1 * m2[3]);
}

NANON_NAMESPACE_END