/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "math.transform.h"
#include "math.basic.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE TMat4<T> Translate(const TMat4<T>& m, const TVec3<T>& v)
{
	TMat4<T> r(m);
	r[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
	return r;
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Translate(const TVec3<T>& v)
{
	return Translate<T>(TMat4<T>::Identity(), v);
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Rotate(const TMat4<T>& m, T angle, const TVec3<T>& axis)
{
	T c = Cos(Radians(angle));
	T s = Sin(Radians(angle));

	TVec3<T> a = Normalize(axis);
	TVec3<T> t = T(T(1) - c) * a;	// For expression template, (T(1) - c) * a generates compile errors

	TMat4<T> rot;
	rot[0][0] = c + t[0] * a[0];
	rot[0][1] =     t[0] * a[1] + s * a[2];
	rot[0][2] =     t[0] * a[2] - s * a[1];
	rot[1][0] =     t[1] * a[0] - s * a[2];
	rot[1][1] = c + t[1] * a[1];
	rot[1][2] =     t[1] * a[2] + s * a[0];
	rot[2][0] =     t[2] * a[0] + s * a[1];
	rot[2][1] =     t[2] * a[1] - s * a[0];
	rot[2][2] = c + t[2] * a[2];

	TMat4<T> r;
	r[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
	r[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
	r[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
	r[3] = m[3];

	return r;
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Rotate(T angle, const TVec3<T>& axis)
{
	return Rotate(TMat4<T>::Identity(), angle, axis);
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Scale(const TMat4<T>& m, const TVec3<T>& v)
{
	return TMat4<T>(m[0] * v[0], m[1] * v[1], m[2] * v[2], m[3]);
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Scale(const TVec3<T>& v)
{
	return Scale(TMat4<T>::Identity(), v);
}

template <typename T>
LM_FORCE_INLINE TMat4<T> LookAt(const TVec3<T>& eye, const TVec3<T>& center, const TVec3<T>& up)
{
	auto f = Normalize(center - eye);
	auto u = Normalize(up);
	auto s = Normalize(Cross(f, u));
	u = Cross(s, f);

	return TMat4<T>(
		s.x, u.x, -f.x, T(0),
		s.y, u.y, -f.y, T(0),
		s.z, u.z, -f.z, T(0),
		-Dot(s, eye), -Dot(u, eye), Dot(f, eye), T(1));
}

template <typename T>
LM_FORCE_INLINE TMat4<T> Perspective(T fovy, T aspect, T zNear, T zFar)
{
	T radian = Radians(fovy);
	T t = Math::Tan(T(radian / T(2)));	// Wrap by T for expression template type

	return TMat4<T>(
		TVec4<T>(T(1) / (aspect * t), T(0), T(0), T(0)),
		TVec4<T>(T(0), T(1) / t, T(0), T(0)),
		TVec4<T>(T(0), T(0), -(zFar + zNear) / (zFar - zNear), -T(1)),
		TVec4<T>(T(0), T(0), -(T(2) * zFar * zNear) / (zFar - zNear), T(0)));
}

// --------------------------------------------------------------------------------

#if LM_SSE2

template <>
LM_FORCE_INLINE Mat4f Rotate(const Mat4f& m, float angle, const Vec3f& axis)
{
	return m * Rotate(angle, axis);
}

template <>
LM_FORCE_INLINE Mat4f Rotate(float angle, const Vec3f& axis)
{
	float c = Cos(Radians(angle));
	float s = Sin(Radians(angle));

	Vec3f a = Normalize(axis);
	Vec3f t = (1.0f - c) * a;

	Mat3f rot(
		Vec3f(c, 0, 0) + a * t[0] + Vec3f( 0       ,  s * a[2], -s * a[1]),
		Vec3f(0, c, 0) + a * t[1] + Vec3f(-s * a[2],  0       ,  s * a[0]),
		Vec3f(0, 0, c) + a * t[2] + Vec3f( s * a[1], -s * a[0],  0       ));

	return Mat4f(rot);
}

#endif

// --------------------------------------------------------------------------------

#if LM_AVX

template <>
LM_FORCE_INLINE Mat4d Rotate(const Mat4d& m, double angle, const Vec3d& axis)
{
	return m * Rotate(angle, axis);
}

template <>
LM_FORCE_INLINE Mat4d Rotate(double angle, const Vec3d& axis)
{
	double c = Cos(Radians(angle));
	double s = Sin(Radians(angle));

	Vec3d a = Normalize(axis);
	Vec3d t = (1.0f - c) * a;

	Mat3d rot(
		Vec3d(c, 0, 0) + a * t[0] + Vec3d( 0       ,  s * a[2], -s * a[1]),
		Vec3d(0, c, 0) + a * t[1] + Vec3d(-s * a[2],  0       ,  s * a[0]),
		Vec3d(0, 0, c) + a * t[2] + Vec3d( s * a[1], -s * a[0],  0       ));

	return Mat4d(rot);
}

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END
