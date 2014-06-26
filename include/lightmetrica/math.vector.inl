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

#include "math.vector.h"
#include "math.basic.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2()
	: x(T(0))
	, y(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2(const T& v)
	: x(v)
	, y(v)
{

}

template <typename T>
LM_FORCE_INLINE TVec2<T>::TVec2(const T& x, const T& y)
	: x(x)
	, y(y)
{

}

template <typename T>
LM_FORCE_INLINE T& TVec2<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE const T& TVec2<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator=(const TVec2<T>& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator+=(const TVec2<T>& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator-=(const TVec2<T>& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator*=(const TVec2<T>& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator/=(const TVec2<T>& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T>& TVec2<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator+(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x + v2.x, v1.y + v2.y);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x - v2.x, v1.y - v2.y);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v, const T& s)
{
	return TVec2<T>(v.x * s, v.y * s);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator*(const T& s, const TVec2<T>& v)
{
	return v * s;
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x * v2.x, v1.y * v2.y);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v, const T& s)
{
	return TVec2<T>(v.x / s, v.y / s);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x / v2.x, v1.y / v2.y);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v)
{
	return TVec2<T>(-v.x, -v.y);
}

template <typename T>
LM_FORCE_INLINE bool operator==(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

template <typename T>
LM_FORCE_INLINE bool operator!=(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return v1.x != v2.x || v1.y != v2.y;
}

template <typename T>
LM_FORCE_INLINE T Length(const TVec2<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
LM_FORCE_INLINE T Length2(const TVec2<T>& v)
{
	return Dot(v, v);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> Normalize(const TVec2<T>& v)
{
	return v / Length(v);
}

template <typename T>
LM_FORCE_INLINE T Dot(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
LM_FORCE_INLINE TVec2<T> Min(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(Min(v1.x, v2.x), Min(v1.y, v2.y));
}

template <typename T>
LM_FORCE_INLINE TVec2<T> Max(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(Max(v1.x, v2.x), Max(v1.y, v2.y));
}

// --------------------------------------------------------------------------------

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3()
	: x(T(0))
	, y(T(0))
	, z(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
	, z(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const T& v)
	: x(v)
	, y(v)
	, z(v)
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const T& x, const T& y, const T& z)
	: x(x)
	, y(y)
	, z(z)
{

}

template <typename T>
LM_FORCE_INLINE TVec3<T>::TVec3(const TVec2<T>& v, const T& z)
	: x(v.x)
	, y(v.y)
	, z(z)
{

}

template <typename T>
LM_FORCE_INLINE T& TVec3<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE const T& TVec3<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator=(const TVec3<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator+=(const TVec3<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator-=(const TVec3<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator*=(const TVec3<T>& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator/=(const TVec3<T>& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T>& TVec3<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator+(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v, const T& s)
{
	return TVec3<T>(v.x * s, v.y * s, v.z * s);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator*(const T& s, const TVec3<T>& v)
{
	return v * s;
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v, const T& s)
{
	return TVec3<T>(v.x / s, v.y / s, v.z / s);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v)
{
	return TVec3<T>(-v.x, -v.y, -v.z);
}

template <typename T>
LM_FORCE_INLINE bool operator==(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

template <typename T>
LM_FORCE_INLINE bool operator!=(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

template <typename T>
LM_FORCE_INLINE T Length(const TVec3<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
LM_FORCE_INLINE T Length2(const TVec3<T>& v)
{
	return Dot(v, v);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> Normalize(const TVec3<T>& v)
{
	return v / Length(v);
}

template <typename T>
LM_FORCE_INLINE T Dot(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
LM_FORCE_INLINE TVec3<T> Cross(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v2.z * v1.x, v1.x * v2.y - v2.x * v1.y);
}

template <typename T>
LM_FORCE_INLINE T LInfinityNorm(const TVec3<T>& v)
{
	return Math::Max(Math::Abs(v.x), Math::Max(Math::Abs(v.y), Math::Abs(v.z)));
}

template <typename T>
LM_FORCE_INLINE TVec3<T> Min(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(Min(v1.x, v2.x), Min(v1.y, v2.y), Min(v1.z, v2.z));
}

template <typename T>
LM_FORCE_INLINE TVec3<T> Max(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(Max(v1.x, v2.x), Max(v1.y, v2.y), Max(v1.z, v2.z));
}

template <typename T>
LM_FORCE_INLINE T Luminance(const TVec3<T>& v)
{
	return
		T(0.212671) * v[0] +
		T(0.715160) * v[1] +
		T(0.072169) * v[2];
}

template <typename T>
LM_FORCE_INLINE bool IsZero(const TVec3<T>& v)
{
	return v.x == T(0) && v.y == T(0) && v.z == T(0);
}

template <typename T>
LM_FORCE_INLINE T CosThetaZUp(const TVec3<T>& v)
{
	return v.z;
}

template <typename T>
LM_FORCE_INLINE T SinTheta2ZUp(const TVec3<T>& v)
{
	return T(1) - v.z - v.z;
}

template <typename T>
LM_FORCE_INLINE T TanThetaZUp(const TVec3<T>& v)
{
	T t = 1 - v.z * v.z;
	return t <= 0 ? 0 : Math::Sqrt(t) / v.z;
}

template <typename T>
LM_FORCE_INLINE TVec3<T> ReflectZUp(const TVec3<T>& wi)
{
	return TVec3<T>(-wi.x, -wi.y, wi.z);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> RefractZUp(const TVec3<T>& wi, const T& eta, const T& cosThetaT)
{
	return TVec3<T>(-eta * wi.x, -eta * wi.y, cosThetaT);
}

// --------------------------------------------------------------------------------

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4()
	: x(T(0))
	, y(T(0))
	, z(T(0))
	, w(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
	, z(T(0))
	, w(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(T(0))
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(v.w)
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const T& v)
	: x(v)
	, y(v)
	, z(v)
	, w(v)
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const T& x, const T& y, const T& z, const T& w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{

}

template <typename T>
LM_FORCE_INLINE TVec4<T>::TVec4(const TVec3<T>& v, const T& w)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(w)
{
	
}

template <typename T>
LM_FORCE_INLINE T& TVec4<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE const T& TVec4<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator=(const TVec4<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator+=(const TVec4<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator-=(const TVec4<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator*=(const TVec4<T>& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator/=(const TVec4<T>& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T>& TVec4<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator+(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v, const T& s)
{
	return TVec4<T>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator*(const T& s, const TVec4<T>& v)
{
	return v * s;
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v, const T& s)
{
	return TVec4<T>(v.x / s, v.y / s, v.z / s, v.w / s);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v)
{
	return TVec4<T>(-v.x, -v.y, -v.z, -v.w);
}

template <typename T>
LM_FORCE_INLINE bool operator==(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

template <typename T>
LM_FORCE_INLINE bool operator!=(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z || v1.w != v2.w;
}

template <typename T>
LM_FORCE_INLINE T Length(const TVec4<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
LM_FORCE_INLINE T Length2(const TVec4<T>& v)
{
	return Dot(v, v);
}

template <typename T>
LM_FORCE_INLINE TVec4<T> Normalize(const TVec4<T>& v)
{
	return v / Length(v);
}

template <typename T>
LM_FORCE_INLINE T Dot(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

template <typename T>
LM_FORCE_INLINE TVec4<T> Min(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(Min(v1.x, v2.x), Min(v1.y, v2.y), Min(v1.z, v2.z), Min(v1.w, v2.w));
}

template <typename T>
LM_FORCE_INLINE TVec4<T> Max(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(Max(v1.x, v2.x), Max(v1.y, v2.y), Max(v1.z, v2.z), Max(v1.w, v2.w));
}

// --------------------------------------------------------------------------------

#if LM_SSE2

LM_FORCE_INLINE Vec3f::TVec3()
	: v(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f))
{

}

LM_FORCE_INLINE Vec3f::TVec3(const Vec2f& v)
	: v(_mm_set_ps(1.0f, 0.0f, v.y, v.x))
{

}

LM_FORCE_INLINE Vec3f::TVec3(const Vec3f& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec3f::TVec3(const Vec4f& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec3f::TVec3(float v)
	: v(_mm_set1_ps(v))
{

}

LM_FORCE_INLINE Vec3f::TVec3(__m128 v)
	: v(v)
{

}

LM_FORCE_INLINE Vec3f::TVec3(float x, float y, float z)
	: v(_mm_set_ps(1.0f, z, y, x))
{

}

LM_FORCE_INLINE Vec3f::TVec3(const Vec2f& v, float z)
	: v(_mm_set_ps(1.0f, z, v.y, v.x))
{

}

LM_FORCE_INLINE float Vec3f::operator[](int i) const
{
	return (&x)[i];
}

LM_FORCE_INLINE Vec3f& Vec3f::operator=(const Vec3f& v)
{
	this->v = v.v;
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator+=(const Vec3f& v)
{
	this->v = _mm_add_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator-=(const Vec3f& v)
{
	this->v = _mm_sub_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator*=(const Vec3f& v)
{
	this->v = _mm_mul_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator*=(float s)
{
	this->v = _mm_mul_ps(this->v, _mm_set1_ps(s));
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator/=(const Vec3f& v)
{
	//this->v = _mm_div_ps(this->v, _mm_blend_ps(v.v, _mm_set1_ps(1.0f), 0x8));
	this->v = _mm_div_ps(this->v, v.v);
	//Vec3f t(v.v);
	//t._ = 1.0f;
	//this->v = _mm_div_ps(this->v, t.v);
	return *this;
}

LM_FORCE_INLINE Vec3f& Vec3f::operator/=(float s)
{
	//this->v = _mm_div_ps(this->v, _mm_set_ps(1.0f, s, s, s));
	this->v = _mm_div_ps(this->v, _mm_set_ps1(s));
	//Vec3f t(s);
	//t._ = 1.0f;
	//this->v = _mm_div_ps(this->v, t.v);
	return *this;
}

template <>
LM_FORCE_INLINE Vec3f operator+(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_add_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3f operator-(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_sub_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3f operator*(const Vec3f& v, const float& s)
{
	return Vec3f(_mm_mul_ps(v.v, _mm_set1_ps(s)));
}

template <>
LM_FORCE_INLINE Vec3f operator*(const float& s, const Vec3f& v)
{
	return v * s;
}

template <>
LM_FORCE_INLINE Vec3f operator*(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_mul_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3f operator/(const Vec3f& v, const float& s)
{
	return v / Vec3f(s);
}

template <>
LM_FORCE_INLINE Vec3f operator/(const Vec3f& v1, const Vec3f& v2)
{
	//return Vec3f(_mm_div_ps(v1.v, _mm_blend_ps(v2.v, _mm_set1_ps(1.0f), 0x8)));
	return Vec3f(_mm_div_ps(v1.v, v2.v));
	//Vec3f t(v2);
	//t._ = 1.0f;
	//return Vec3f(_mm_div_ps(v1.v, t.v));
}

template <>
LM_FORCE_INLINE Vec3f operator-(const Vec3f& v)
{
	return Vec3f(_mm_sub_ps(_mm_setzero_ps(), v.v));
}

template <>
LM_FORCE_INLINE float Length(const Vec3f& v)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0x71)));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE float Length2(const Vec3f& v)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v.v, v.v, 0x71));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE Vec3f Normalize(const Vec3f& v)
{
#if LM_SSE4_1
	return Vec3f(_mm_mul_ps(v.v, _mm_rsqrt_ps(_mm_dp_ps(v.v, v.v, 0x7f))));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE float Dot(const Vec3f& v1, const Vec3f& v2)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v1.v, v2.v, 0x71));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE Vec3f Cross(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(
		_mm_sub_ps(
			_mm_mul_ps(
				_mm_shuffle_ps(v1.v, v1.v, _MM_SHUFFLE(3, 0, 2, 1)),
				_mm_shuffle_ps(v2.v, v2.v, _MM_SHUFFLE(3, 1, 0, 2))), 
			_mm_mul_ps(
				_mm_shuffle_ps(v1.v, v1.v, _MM_SHUFFLE(3, 1, 0, 2)),
				_mm_shuffle_ps(v2.v, v2.v, _MM_SHUFFLE(3, 0, 2, 1)))));
}

template <>
LM_FORCE_INLINE float LInfinityNorm(const Vec3f& v)
{
	// Abs. Note: v_abs.z = 0
	static const __m128 Mask = _mm_castsi128_ps(_mm_set1_epi32(0x800000ff));
	__m128 v_abs = _mm_andnot_ps(Mask, v.v);

	// Horizontal max
	__m128 v_1032 = _mm_shuffle_ps(v_abs, v_abs, _MM_SHUFFLE(1, 0, 3, 2));
	__m128 v2 = _mm_max_ps(v_abs, v_1032);
	__m128 v2_2301 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 v_max = _mm_max_ps(v2, v2_2301);

	return _mm_cvtss_f32(v_max);
}

template <>
LM_FORCE_INLINE Vec3f Min(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_min_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3f Max(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_max_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE bool IsZero(const Vec3f& v)
{
	return (_mm_movemask_ps(_mm_cmpeq_ps(v.v, _mm_setzero_ps())) & 0x7) == 7;
}

// --------------------------------------------------------------------------------

LM_FORCE_INLINE Vec4f::TVec4()
	: v(_mm_setzero_ps())
{

}

LM_FORCE_INLINE Vec4f::TVec4(const Vec2f& v)
	: v(_mm_set_ps(0.0f, 0.0f, v.y, v.x))
{

}

LM_FORCE_INLINE Vec4f::TVec4(const Vec3f& v)
	: v(_mm_blend_ps(v.v, _mm_setzero_ps(), 0x8))
{

}

LM_FORCE_INLINE Vec4f::TVec4(const Vec4f& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec4f::TVec4(float v)
	: v(_mm_set1_ps(v))
{

}

LM_FORCE_INLINE Vec4f::TVec4(__m128 v)
	: v(v)
{

}

LM_FORCE_INLINE Vec4f::TVec4(float x, float y, float z, float w)
	: v(_mm_set_ps(w, z, y, x))
{

}

LM_FORCE_INLINE Vec4f::TVec4(const Vec3f& v, float w)
{
	this->v = v.v;
	this->w = w;
}

LM_FORCE_INLINE float Vec4f::operator[](int i) const
{
	return (&x)[i];
}

LM_FORCE_INLINE Vec4f& Vec4f::operator=(const Vec4f& v)
{
	this->v = v.v;
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator+=(const Vec4f& v)
{
	this->v = _mm_add_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator-=(const Vec4f& v)
{
	this->v = _mm_sub_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator*=(const Vec4f& v)
{
	this->v = _mm_mul_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator*=(float s)
{
	this->v = _mm_mul_ps(this->v, _mm_set1_ps(s));
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator/=(const Vec4f& v)
{
	this->v = _mm_div_ps(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4f& Vec4f::operator/=(float s)
{
	this->v = _mm_div_ps(this->v, _mm_set1_ps(s));
	return *this;
}

template <>
LM_FORCE_INLINE Vec4f operator+(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_add_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4f operator-(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_sub_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4f operator*(const Vec4f& v, const float& s)
{
	return Vec4f(_mm_mul_ps(v.v, _mm_set1_ps(s)));
}

template <>
LM_FORCE_INLINE Vec4f operator*(const float& s, const Vec4f& v)
{
	return v * s;
}

template <>
LM_FORCE_INLINE Vec4f operator*(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_mul_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4f operator/(const Vec4f& v, const float& s)
{
	return v / Vec4f(s);
}

template <>
LM_FORCE_INLINE Vec4f operator/(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_div_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4f operator-(const Vec4f& v)
{
	return Vec4f(_mm_sub_ps(_mm_setzero_ps(), v.v));
}

template <>
LM_FORCE_INLINE float Length(const Vec4f& v)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0xf1)));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE float Length2(const Vec4f& v)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v.v, v.v, 0xf1));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE Vec4f Normalize(const Vec4f& v)
{
#if LM_SSE4_1
	return Vec4f(_mm_mul_ps(v.v, _mm_rsqrt_ps(_mm_dp_ps(v.v, v.v, 0xff))));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE float Dot(const Vec4f& v1, const Vec4f& v2)
{
#if LM_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v1.v, v2.v, 0xf1));
#else
#error "TODO"
#endif
}

template <>
LM_FORCE_INLINE Vec4f Min(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_min_ps(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4f Max(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_max_ps(v1.v, v2.v));
}

#endif

// --------------------------------------------------------------------------------

#if LM_AVX

LM_FORCE_INLINE Vec3d::TVec3()
	: v(_mm256_set_pd(1, 0, 0, 0))
{

}

LM_FORCE_INLINE Vec3d::TVec3(const Vec2d& v)
	: v(_mm256_set_pd(1, 0, v.y, v.x))
{

}

LM_FORCE_INLINE Vec3d::TVec3(const Vec3d& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec3d::TVec3(const Vec4d& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec3d::TVec3(double v)
	: v(_mm256_set1_pd(v))
{

}

LM_FORCE_INLINE Vec3d::TVec3(__m256d v)
	: v(v)
{

}

LM_FORCE_INLINE Vec3d::TVec3(double x, double y, double z)
	: v(_mm256_set_pd(1, z, y, x))
{

}

LM_FORCE_INLINE Vec3d::TVec3(const Vec2d& v, double z)
	: v(_mm256_set_pd(1, z, v.y, v.x))
{

}

LM_FORCE_INLINE double Vec3d::operator[](int i) const
{
	return (&x)[i];
}

LM_FORCE_INLINE Vec3d& Vec3d::operator=(const Vec3d& v)
{
	this->v = v.v;
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator+=(const Vec3d& v)
{
	this->v = _mm256_add_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator-=(const Vec3d& v)
{
	this->v = _mm256_sub_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator*=(const Vec3d& v)
{
	this->v = _mm256_mul_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator*=(double s)
{
	this->v = _mm256_mul_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator/=(const Vec3d& v)
{
	this->v = _mm256_div_pd(this->v, v.v);
	//Vec3d t(v.v);
	//t._ = 1.0f;
	//this->v = _mm256_div_pd(this->v, t.v);
	return *this;
}

LM_FORCE_INLINE Vec3d& Vec3d::operator/=(double s)
{
	this->v = _mm256_div_pd(this->v, _mm256_set1_pd(s));
	//Vec3d t(s);
	//t._ = 1.0f;
	//this->v = _mm256_div_pd(this->v, t.v);
	return *this;
}

template <>
LM_FORCE_INLINE Vec3d operator+(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_add_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3d operator-(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_sub_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3d operator*(const Vec3d& v, const double& s)
{
	return Vec3d(_mm256_mul_pd(v.v, _mm256_set1_pd(s)));
}

template <>
LM_FORCE_INLINE Vec3d operator*(const double& s, const Vec3d& v)
{
	return v * s;
}

template <>
LM_FORCE_INLINE Vec3d operator*(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_mul_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3d operator/(const Vec3d& v, const double& s)
{
	return v / Vec3d(s);
}

template <>
LM_FORCE_INLINE Vec3d operator/(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_div_pd(v1.v, v2.v));
	//Vec3d t(v2);
	//t._ = 1.0f;
	//return Vec3d(_mm256_div_pd(v1.v, t.v));
}

template <>
LM_FORCE_INLINE Vec3d operator-(const Vec3d& v)
{
	return Vec3d(_mm256_sub_pd(_mm256_setzero_pd(), v.v));
}

template <>
LM_FORCE_INLINE double Length(const Vec3d& v)
{
	return Math::Sqrt(Length2(v));
}

template <>
LM_FORCE_INLINE double Length2(const Vec3d& v)
{
	return Dot(v, v);
}

template <>
LM_FORCE_INLINE Vec3d Normalize(const Vec3d& v)
{
	return v / Length(v);
}

template <>
LM_FORCE_INLINE double Dot(const Vec3d& v1, const Vec3d& v2)
{
	__m256d z = _mm256_setzero_pd();
	__m256d tv1 = _mm256_blend_pd(v1.v, z, 0x8);	// = ( 0, z1, y1, x1 )
	__m256d tv2 = _mm256_blend_pd(v2.v, z, 0x8);	// = ( 0, z2, y2, x2 )
	__m256d t1 = _mm256_mul_pd(tv1, tv2);			// = ( 0, z1 * z2, y1 * y2, x1 * x2 )
	__m256d t2 = _mm256_hadd_pd(t1, t1);			// = ( z1 * z2, z1 * z2, x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
	__m128d t3 = _mm256_extractf128_pd(t2, 1);		// = ( z1 * z2, z1 * z2 )
	__m128d t4 = _mm256_castpd256_pd128(t2);		// = ( x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
	__m128d result = _mm_add_pd(t3, t4);			// = ( x1 * x2 + y1 * y2 + z1 * z2, x1 * x2 + y1 * y2 + z1 * z2 )
	return _mm_cvtsd_f64(result);
}

template <>
LM_FORCE_INLINE Vec3d Cross(const Vec3d& v1, const Vec3d& v2)
{
#if LM_AVX2

	return Vec3d(
		_mm256_sub_pd(
			_mm256_mul_pd(
				_mm256_permute4x64_pd(v1.v, _MM_SHUFFLE(3, 0, 2, 1)),
				_mm256_permute4x64_pd(v2.v, _MM_SHUFFLE(3, 1, 0, 2))), 
			_mm256_mul_pd(
				_mm256_permute4x64_pd(v1.v, _MM_SHUFFLE(3, 1, 0, 2)),
				_mm256_permute4x64_pd(v2.v, _MM_SHUFFLE(3, 0, 2, 1)))));

#else

	__m256d v1_1032 = _mm256_permute2f128_pd(v1.v, v1.v, 1);
	__m256d v1_0321 = _mm256_shuffle_pd(v1.v, v1_1032, 0x5);
	__m256d v1_3021 = _mm256_permute_pd(v1_0321, 0x6);
	__m256d v1_1320 = _mm256_shuffle_pd(v1.v, v1_1032, 0xc);
	__m256d v1_3102 = _mm256_permute_pd(v1_1320, 0x5);

	__m256d v2_1032 = _mm256_permute2f128_pd(v2.v, v2.v, 1);
	__m256d v2_0321 = _mm256_shuffle_pd(v2.v, v2_1032, 0x5);
	__m256d v2_3021 = _mm256_permute_pd(v2_0321, 0x6);
	__m256d v2_1320 = _mm256_shuffle_pd(v2.v, v2_1032, 0xc);
	__m256d v2_3102 = _mm256_permute_pd(v2_1320, 0x5);

	return Vec3d(
		_mm256_sub_pd(
			_mm256_mul_pd(v1_3021, v2_3102), 
			_mm256_mul_pd(v1_3102, v2_3021)));

#endif
}

template <>
LM_FORCE_INLINE double LInfinityNorm(const Vec3d& v)
{
	// Abs. Note: v_abs.z = 0
	static const __m256d Mask = _mm256_castsi256_pd(_mm256_set1_epi64x(0x800000000000ffff));
	__m256d v_abs = _mm256_andnot_pd(Mask, v.v);

	// Horizontal max
	__m256d v_1032 = _mm256_permute2f128_pd(v_abs, v_abs, 1);
	__m256d v2 = _mm256_max_pd(v_abs, v_1032);
	__m256d v2_2301 = _mm256_permute_pd(v2, 0x5);
	__m256d v_max = _mm256_max_pd(v2, v2_2301);

	return _mm_cvtsd_f64(_mm256_castpd256_pd128(v_max));
}

template <>
LM_FORCE_INLINE Vec3d Min(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_min_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec3d Max(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_max_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE bool IsZero(const Vec3d& v)
{
	return (_mm256_movemask_pd(_mm256_cmp_pd(v.v, _mm256_setzero_pd(), _CMP_EQ_OQ)) & 0x7) == 7;
}

// --------------------------------------------------------------------------------

LM_FORCE_INLINE Vec4d::TVec4()
	: v(_mm256_setzero_pd())
{

}

LM_FORCE_INLINE Vec4d::TVec4(const Vec2d& v)
	: v(_mm256_set_pd(0, 0, v.y, v.x))
{

}

LM_FORCE_INLINE Vec4d::TVec4(const Vec3d& v)
	: v(_mm256_blend_pd(v.v, _mm256_setzero_pd(), 0x8))
{

}

LM_FORCE_INLINE Vec4d::TVec4(const Vec4d& v)
	: v(v.v)
{

}

LM_FORCE_INLINE Vec4d::TVec4(double v)
	: v(_mm256_set1_pd(v))
{

}

LM_FORCE_INLINE Vec4d::TVec4(__m256d v)
	: v(v)
{

}

LM_FORCE_INLINE Vec4d::TVec4(double x, double y, double z, double w)
	: v(_mm256_set_pd(w, z, y, x))
{

}

LM_FORCE_INLINE Vec4d::TVec4(const Vec3d& v, double w)
{
	this->v = v.v;
	this->w = w;
}

LM_FORCE_INLINE double Vec4d::operator[](int i) const
{
	return (&x)[i];
}

LM_FORCE_INLINE Vec4d& Vec4d::operator=(const Vec4d& v)
{
	this->v = v.v;
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator+=(const Vec4d& v)
{
	this->v = _mm256_add_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator-=(const Vec4d& v)
{
	this->v = _mm256_sub_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator*=(const Vec4d& v)
{
	this->v = _mm256_mul_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator*=(double s)
{
	this->v = _mm256_mul_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator/=(const Vec4d& v)
{
	this->v = _mm256_div_pd(this->v, v.v);
	return *this;
}

LM_FORCE_INLINE Vec4d& Vec4d::operator/=(double s)
{
	this->v = _mm256_div_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

template <>
LM_FORCE_INLINE Vec4d operator+(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_add_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4d operator-(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_sub_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4d operator*(const Vec4d& v, const double& s)
{
	return Vec4d(_mm256_mul_pd(v.v, _mm256_set1_pd(s)));
}

template <>
LM_FORCE_INLINE Vec4d operator*(const double& s, const Vec4d& v)
{
	return v * s;
}

template <>
LM_FORCE_INLINE Vec4d operator*(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_mul_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4d operator/(const Vec4d& v, const double& s)
{
	return v / Vec4d(s);
}

template <>
LM_FORCE_INLINE Vec4d operator/(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_div_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4d operator-(const Vec4d& v)
{
	return Vec4d(_mm256_sub_pd(_mm256_setzero_pd(), v.v));
}

template <>
LM_FORCE_INLINE double Length(const Vec4d& v)
{
	return Math::Sqrt(Length2(v));
}

template <>
LM_FORCE_INLINE double Length2(const Vec4d& v)
{
	return Dot(v, v);
}

template <>
LM_FORCE_INLINE Vec4d Normalize(const Vec4d& v)
{
	return v / Length(v);
}

template <>
LM_FORCE_INLINE double Dot(const Vec4d& v1, const Vec4d& v2)
{
	__m256d t1 = _mm256_mul_pd(v1.v, v2.v);		// = ( w1 * w2, z1 * z2, y1 * y2, x1 * x2 )
	__m256d t2 = _mm256_hadd_pd(t1, t1);		// = ( z1 * z2 + w1 * w2, z1 * z2 + w1 * w2, x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
	__m128d t3 = _mm256_extractf128_pd(t2, 1);	// = ( z1 * z2 + w1 * w2, z1 * z2 + w1 * w2 )
	__m128d t4 = _mm256_castpd256_pd128(t2);	// = ( x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
	__m128d result = _mm_add_pd(t3, t4);		// = ( _, v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w )
	return _mm_cvtsd_f64(result);
}

template <>
LM_FORCE_INLINE Vec4d Min(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_min_pd(v1.v, v2.v));
}

template <>
LM_FORCE_INLINE Vec4d Max(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_max_pd(v1.v, v2.v));
}

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END