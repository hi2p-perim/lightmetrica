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

#include "mathfunctions.h"

NANON_NAMESPACE_BEGIN

namespace MathFunc
{

template <typename T>
NANON_FORCE_INLINE Mat4<T> Translate(const Mat4<T>& m, const Vec3<T>& v)
{
	Mat4<T> r(m);
	r[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
	return r;
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Translate(const Vec3<T>& v)
{
	return Translate<T>(Mat4d::Identity(), v);
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Rotate(const Mat4<T>& m, T angle, const Vec3<T>& axis)
{
	T c = std::cos(Radians(angle));
	T s = std::sin(Radians(angle));

	auto a = Normalize(axis);
	auto t = (T(1) - c) * a;

	Mat4<T> rot;
	rot[0][0] = c + t[0] * a[0];
	rot[0][1] = 0 + t[0] * a[1] + s * a[2];
	rot[0][2] = 0 + t[0] * a[2] - s * a[1];
	rot[1][0] = 0 + t[1] * a[0] - s * a[2];
	rot[1][1] = c + t[1] * a[1];
	rot[1][2] = 0 + t[1] * a[2] + s * a[0];
	rot[2][0] = 0 + t[2] * a[0] + s * a[1];
	rot[2][1] = 0 + t[2] * a[1] - s * a[0];
	rot[2][2] = c + t[2] * a[2];

	Mat4<T> r;
	r[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
	r[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
	r[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
	r[3] = m[3];

	return r;
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Rotate(T angle, const Vec3<T>& axis)
{
	return Rotate(Mat4<T>::Identity(), angle, axis);
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Scale(const Mat4<T>& m, const Vec3<T>& v)
{
	return Mat4<T>(m[0] * v[0], m[1] * v[1], m[2] * v[2], m[3]);
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Scale(const Vec3<T>& v)
{
	return Scale(Mat4<T>::Identity(), v);
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> LookAt(const Vec3<T>& eye, const Vec3<T>& center, const Vec3<T>& up)
{
	auto f = Normalize(center - eye);
	auto u = Normalize(up);
	auto s = Normalize(Cross(f, u));
	u = Cross(s, f);

	auto r = Mat4<T>::Identity();
	r[0][0] = s.x;
	r[1][0] = s.y;
	r[2][0] = s.z;
	r[0][1] = u.x;
	r[1][1] = u.y;
	r[2][1] = u.z;
	r[0][2] = -f.x;
	r[1][2] = -f.y;
	r[2][2] = -f.z;
	r[3][0] = -Dot(s, eye);
	r[3][1] = -Dot(u, eye);
	r[3][2] = Dot(f, eye);

	return r;
}

template <typename T>
NANON_FORCE_INLINE Mat4<T> Perspective(T fovy, T aspect, T zNear, T zFar)
{
	T radian = Radians(fovy);
	T t = std::tan(radian / T(2));

	Mat4<T> r;
	r[0][0] = T(1) / (aspect * t);
	r[1][1] = T(1) / t;
	r[2][2] = -(zFar + zNear) / (zFar - zNear);
	r[2][3] = -T(1);
	r[3][2] = -(T(2) * zFar * zNear) / (zFar - zNear);

	return r;
}

}

NANON_NAMESPACE_END