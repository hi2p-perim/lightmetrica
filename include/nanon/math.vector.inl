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

#include "math.vector.h"
#include "math.basic.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2()
	: x(T(0))
	, y(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
{

}

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2(const T& v)
	: x(v)
	, y(v)
{

}

template <typename T>
NANON_FORCE_INLINE TVec2<T>::TVec2(const T& x, const T& y)
	: x(x)
	, y(y)
{

}

template <typename T>
NANON_FORCE_INLINE T& TVec2<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE const T& TVec2<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator=(const TVec2<T>& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator+=(const TVec2<T>& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator-=(const TVec2<T>& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator*=(const TVec2<T>& v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator/=(const TVec2<T>& v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T>& TVec2<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator+(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x + v2.x, v1.y + v2.y);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x - v2.x, v1.y - v2.y);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v, const T& s)
{
	return TVec2<T>(v.x * s, v.y * s);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator*(const T& s, const TVec2<T>& v)
{
	return v * s;
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator*(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x * v2.x, v1.y * v2.y);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v, const T& s)
{
	return TVec2<T>(v.x / s, v.y / s);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator/(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return TVec2<T>(v1.x / v2.x, v1.y / v2.y);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> operator-(const TVec2<T>& v)
{
	return TVec2<T>(-v.x, -v.y);
}

template <typename T>
NANON_FORCE_INLINE T Length(const TVec2<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
NANON_FORCE_INLINE T Length2(const TVec2<T>& v)
{
	return Dot(v, v);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> Normalize(const TVec2<T>& v)
{
	return v / Length(v);
}

template <typename T>
NANON_FORCE_INLINE T Dot(const TVec2<T>& v1, const TVec2<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

// --------------------------------------------------------------------------------

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3()
	: x(T(0))
	, y(T(0))
	, z(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
	, z(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const T& v)
	: x(v)
	, y(v)
	, z(v)
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const T& x, const T& y, const T& z)
	: x(x)
	, y(y)
	, z(z)
{

}

template <typename T>
NANON_FORCE_INLINE TVec3<T>::TVec3(const TVec2<T>& v, const T& z)
	: x(v.x)
	, y(v.y)
	, z(z)
{

}

template <typename T>
NANON_FORCE_INLINE T& TVec3<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE const T& TVec3<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator=(const TVec3<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator+=(const TVec3<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator-=(const TVec3<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator*=(const TVec3<T>& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator/=(const TVec3<T>& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T>& TVec3<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator+(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v, const T& s)
{
	return TVec3<T>(v.x * s, v.y * s, v.z * s);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator*(const T& s, const TVec3<T>& v)
{
	return v * s;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator*(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v, const T& s)
{
	return TVec3<T>(v.x / s, v.y / s, v.z / s);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator/(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> operator-(const TVec3<T>& v)
{
	return TVec3<T>(-v.x, -v.y, -v.z);
}

template <typename T>
NANON_FORCE_INLINE T Length(const TVec3<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
NANON_FORCE_INLINE T Length2(const TVec3<T>& v)
{
	return Dot(v, v);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> Normalize(const TVec3<T>& v)
{
	return v / Length(v);
}

template <typename T>
NANON_FORCE_INLINE T Dot(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> Cross(const TVec3<T>& v1, const TVec3<T>& v2)
{
	return TVec3<T>(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v2.z * v1.x, v1.x * v2.y - v2.x * v1.y);
}

// --------------------------------------------------------------------------------

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4()
	: x(T(0))
	, y(T(0))
	, z(T(0))
	, w(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const TVec2<T>& v)
	: x(v.x)
	, y(v.y)
	, z(T(0))
	, w(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const TVec3<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(T(0))
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const TVec4<T>& v)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(v.w)
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const T& v)
	: x(v)
	, y(v)
	, z(v)
	, w(v)
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const T& x, const T& y, const T& z, const T& w)
	: x(x)
	, y(y)
	, z(z)
	, w(w)
{

}

template <typename T>
NANON_FORCE_INLINE TVec4<T>::TVec4(const TVec3<T>& v, const T& w)
	: x(v.x)
	, y(v.y)
	, z(v.z)
	, w(w)
{
	
}

template <typename T>
NANON_FORCE_INLINE T& TVec4<T>::operator[](int i)
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE const T& TVec4<T>::operator[](int i) const
{
	return (&x)[i];
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator=(const TVec4<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator+=(const TVec4<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator-=(const TVec4<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator*=(const TVec4<T>& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator*=(const T& s)
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator/=(const TVec4<T>& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T>& TVec4<T>::operator/=(const T& s)
{
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator+(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v, const T& s)
{
	return TVec4<T>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator*(const T& s, const TVec4<T>& v)
{
	return v * s;
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator*(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v, const T& s)
{
	return TVec4<T>(v.x / s, v.y / s, v.z / s, v.w / s);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator/(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return TVec4<T>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> operator-(const TVec4<T>& v)
{
	return TVec4<T>(-v.x, -v.y, -v.z, -v.w);
}

template <typename T>
NANON_FORCE_INLINE T Length(const TVec4<T>& v)
{
	return Math::Sqrt(Math::Length2(v));
}

template <typename T>
NANON_FORCE_INLINE T Length2(const TVec4<T>& v)
{
	return Dot(v, v);
}

template <typename T>
NANON_FORCE_INLINE TVec4<T> Normalize(const TVec4<T>& v)
{
	return v / Length(v);
}

template <typename T>
NANON_FORCE_INLINE T Dot(const TVec4<T>& v1, const TVec4<T>& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

// --------------------------------------------------------------------------------

#ifdef NANON_USE_SSE2

NANON_FORCE_INLINE Vec3f::TVec3()
	: v(_mm_setzero_ps())
{

}

NANON_FORCE_INLINE Vec3f::TVec3(const Vec2f& v)
	: v(_mm_set_ps(0.0f, 0.0f, v.y, v.x))
{

}

NANON_FORCE_INLINE Vec3f::TVec3(const Vec3f& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec3f::TVec3(const Vec4f& v)
	: v(v.v)
{
	_ = 0.0f;
}

NANON_FORCE_INLINE Vec3f::TVec3(float v)
	: v(_mm_set1_ps(v))
{

}

NANON_FORCE_INLINE Vec3f::TVec3(__m128 v)
	: v(v)
{

}

NANON_FORCE_INLINE Vec3f::TVec3(float x, float y, float z)
	: v(_mm_set_ps(0.0f, z, y, x))
{

}

NANON_FORCE_INLINE Vec3f::TVec3(const Vec2f& v, float z)
	: v(_mm_set_ps(0.0f, z, v.y, v.x))
{

}

NANON_FORCE_INLINE float Vec3f::operator[](int i) const
{
	return (&x)[i];
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator=(const Vec3f& v)
{
	this->v = v.v;
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator+=(const Vec3f& v)
{
	this->v = _mm_add_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator-=(const Vec3f& v)
{
	this->v = _mm_sub_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator*=(const Vec3f& v)
{
	this->v = _mm_mul_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator*=(float s)
{
	this->v = _mm_mul_ps(this->v, _mm_set1_ps(s));
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator/=(const Vec3f& v)
{
	Vec3f t(v.v);
	t._ = 1.0f;
	this->v = _mm_div_ps(this->v, t.v);
	return *this;
}

NANON_FORCE_INLINE Vec3f& Vec3f::operator/=(float s)
{
	Vec3f t(s);
	t._ = 1.0f;
	this->v = _mm_div_ps(this->v, t.v);
	return *this;
}

template <>
NANON_FORCE_INLINE Vec3f operator+(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_add_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3f operator-(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_sub_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3f operator*(const Vec3f& v, const float& s)
{
	return Vec3f(_mm_mul_ps(v.v, _mm_set1_ps(s)));
}

template <>
NANON_FORCE_INLINE Vec3f operator*(const float& s, const Vec3f& v)
{
	return v * s;
}

template <>
NANON_FORCE_INLINE Vec3f operator*(const Vec3f& v1, const Vec3f& v2)
{
	return Vec3f(_mm_mul_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3f operator/(const Vec3f& v, const float& s)
{
	return v / Vec3f(s);
}

template <>
NANON_FORCE_INLINE Vec3f operator/(const Vec3f& v1, const Vec3f& v2)
{
	Vec3f t(v2);
	t._ = 1.0f;
	return Vec3f(_mm_div_ps(v1.v, t.v));
}

template <>
NANON_FORCE_INLINE Vec3f operator-(const Vec3f& v)
{
	return Vec3f(_mm_sub_ps(_mm_setzero_ps(), v.v));
}

template <>
NANON_FORCE_INLINE float Length(const Vec3f& v)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0xf1)));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE float Length2(const Vec3f& v)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v.v, v.v, 0xf1));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE Vec3f Normalize(const Vec3f& v)
{
#ifdef NANON_USE_SSE4_1
	return Vec3f(_mm_mul_ps(v.v, _mm_rsqrt_ps(_mm_dp_ps(v.v, v.v, 0xff))));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE float Dot(const Vec3f& v1, const Vec3f& v2)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v1.v, v2.v, 0xf1));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE Vec3f Cross(const Vec3f& v1, const Vec3f& v2)
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

// --------------------------------------------------------------------------------

NANON_FORCE_INLINE Vec4f::TVec4()
	: v(_mm_setzero_ps())
{

}

NANON_FORCE_INLINE Vec4f::TVec4(const Vec2f& v)
	: v(_mm_set_ps(0.0f, 0.0f, v.y, v.x))
{

}

NANON_FORCE_INLINE Vec4f::TVec4(const Vec3f& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec4f::TVec4(const Vec4f& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec4f::TVec4(float v)
	: v(_mm_set1_ps(v))
{

}

NANON_FORCE_INLINE Vec4f::TVec4(__m128 v)
	: v(v)
{

}

NANON_FORCE_INLINE Vec4f::TVec4(float x, float y, float z, float w)
	: v(_mm_set_ps(w, z, y, x))
{

}

NANON_FORCE_INLINE Vec4f::TVec4(const Vec3f& v, float w)
{
	this->v = v.v;
	this->w = w;
}

NANON_FORCE_INLINE float Vec4f::operator[](int i) const
{
	return (&x)[i];
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator=(const Vec4f& v)
{
	this->v = v.v;
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator+=(const Vec4f& v)
{
	this->v = _mm_add_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator-=(const Vec4f& v)
{
	this->v = _mm_sub_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator*=(const Vec4f& v)
{
	this->v = _mm_mul_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator*=(float s)
{
	this->v = _mm_mul_ps(this->v, _mm_set1_ps(s));
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator/=(const Vec4f& v)
{
	this->v = _mm_div_ps(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4f& Vec4f::operator/=(float s)
{
	this->v = _mm_div_ps(this->v, _mm_set1_ps(s));
	return *this;
}

template <>
NANON_FORCE_INLINE Vec4f operator+(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_add_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4f operator-(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_sub_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4f operator*(const Vec4f& v, const float& s)
{
	return Vec4f(_mm_mul_ps(v.v, _mm_set1_ps(s)));
}

template <>
NANON_FORCE_INLINE Vec4f operator*(const float& s, const Vec4f& v)
{
	return v * s;
}

template <>
NANON_FORCE_INLINE Vec4f operator*(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_mul_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4f operator/(const Vec4f& v, const float& s)
{
	return v / Vec4f(s);
}

template <>
NANON_FORCE_INLINE Vec4f operator/(const Vec4f& v1, const Vec4f& v2)
{
	return Vec4f(_mm_div_ps(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4f operator-(const Vec4f& v)
{
	return Vec4f(_mm_sub_ps(_mm_setzero_ps(), v.v));
}

template <>
NANON_FORCE_INLINE float Length(const Vec4f& v)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0xf1)));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE float Length2(const Vec4f& v)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v.v, v.v, 0xf1));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE Vec4f Normalize(const Vec4f& v)
{
#ifdef NANON_USE_SSE4_1
	return Vec4f(_mm_mul_ps(v.v, _mm_rsqrt_ps(_mm_dp_ps(v.v, v.v, 0xff))));
#else
#error "TODO"
#endif
}

template <>
NANON_FORCE_INLINE float Dot(const Vec4f& v1, const Vec4f& v2)
{
#ifdef NANON_USE_SSE4_1
	return _mm_cvtss_f32(_mm_dp_ps(v1.v, v2.v, 0xf1));
#else
#error "TODO"
#endif
}

#endif

// --------------------------------------------------------------------------------

#ifdef NANON_USE_AVX

NANON_FORCE_INLINE Vec3d::TVec3()
	: v(_mm256_setzero_pd())
{

}

NANON_FORCE_INLINE Vec3d::TVec3(const Vec2d& v)
	: v(_mm256_set_pd(0, 0, v.y, v.x))
{

}

NANON_FORCE_INLINE Vec3d::TVec3(const Vec3d& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec3d::TVec3(const Vec4d& v)
	: v(v.v)
{
	_ = 0;
}

NANON_FORCE_INLINE Vec3d::TVec3(double v)
	: v(_mm256_set1_pd(v))
{

}

NANON_FORCE_INLINE Vec3d::TVec3(__m256d v)
	: v(v)
{

}

NANON_FORCE_INLINE Vec3d::TVec3(double x, double y, double z)
	: v(_mm256_set_pd(0, z, y, x))
{

}

NANON_FORCE_INLINE Vec3d::TVec3(const Vec2d& v, double z)
	: v(_mm256_set_pd(0, z, v.y, v.x))
{

}

NANON_FORCE_INLINE double Vec3d::operator[](int i) const
{
	return (&x)[i];
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator=(const Vec3d& v)
{
	this->v = v.v;
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator+=(const Vec3d& v)
{
	this->v = _mm256_add_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator-=(const Vec3d& v)
{
	this->v = _mm256_sub_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator*=(const Vec3d& v)
{
	this->v = _mm256_mul_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator*=(double s)
{
	this->v = _mm256_mul_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator/=(const Vec3d& v)
{
	Vec3d t(v.v);
	t._ = 1.0f;
	this->v = _mm256_div_pd(this->v, t.v);
	return *this;
}

NANON_FORCE_INLINE Vec3d& Vec3d::operator/=(double s)
{
	Vec3d t(s);
	t._ = 1.0f;
	this->v = _mm256_div_pd(this->v, t.v);
	return *this;
}

template <>
NANON_FORCE_INLINE Vec3d operator+(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_add_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3d operator-(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_sub_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3d operator*(const Vec3d& v, const double& s)
{
	return Vec3d(_mm256_mul_pd(v.v, _mm256_set1_pd(s)));
}

template <>
NANON_FORCE_INLINE Vec3d operator*(const double& s, const Vec3d& v)
{
	return v * s;
}

template <>
NANON_FORCE_INLINE Vec3d operator*(const Vec3d& v1, const Vec3d& v2)
{
	return Vec3d(_mm256_mul_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec3d operator/(const Vec3d& v, const double& s)
{
	return v / Vec3d(s);
}

template <>
NANON_FORCE_INLINE Vec3d operator/(const Vec3d& v1, const Vec3d& v2)
{
	Vec3d t(v2);
	t._ = 1.0f;
	return Vec3d(_mm256_div_pd(v1.v, t.v));
}

template <>
NANON_FORCE_INLINE Vec3d operator-(const Vec3d& v)
{
	return Vec3d(_mm256_sub_pd(_mm256_setzero_pd(), v.v));
}

template <>
NANON_FORCE_INLINE double Length(const Vec3d& v)
{
	return Math::Sqrt(Length2(v));
}

template <>
NANON_FORCE_INLINE double Length2(const Vec3d& v)
{
	return Dot(v, v);
}

template <>
NANON_FORCE_INLINE Vec3d Normalize(const Vec3d& v)
{
	return v / Length(v);
}

template <>
NANON_FORCE_INLINE double Dot(const Vec3d& v1, const Vec3d& v2)
{
	__m256d t1 = _mm256_mul_pd(v1.v, v2.v);		// t1 = [ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w ]
	__m256d t2 = _mm256_hadd_pd(t1, t1);		// t2 = [ v1.x * v2.x + v1.y * v2.y, _, v1.z * v2.z + v1.w * v2.w, _ ]
	__m128d t3 = _mm256_extractf128_pd(t2, 1);	// t3 = [ v1.x * v2.x + v1.y * v2.y, _ ]
	__m128d t4 = _mm256_castpd256_pd128(t2);	// t4 = [ v1.z * v2.z + v1.w * v2.w, _ ]
	__m128d result = _mm_add_pd(t3, t4);		// result = [ v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w, _ ]
	return Vec3d(_mm256_castpd128_pd256(result)).x;
}

template <>
NANON_FORCE_INLINE Vec3d Cross(const Vec3d& v1, const Vec3d& v2)
{
#ifdef NANON_USE_AVX2

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

// --------------------------------------------------------------------------------

NANON_FORCE_INLINE Vec4d::TVec4()
	: v(_mm256_setzero_pd())
{

}

NANON_FORCE_INLINE Vec4d::TVec4(const Vec2d& v)
	: v(_mm256_set_pd(0, 0, v.y, v.x))
{

}

NANON_FORCE_INLINE Vec4d::TVec4(const Vec3d& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec4d::TVec4(const Vec4d& v)
	: v(v.v)
{

}

NANON_FORCE_INLINE Vec4d::TVec4(double v)
	: v(_mm256_set1_pd(v))
{

}

NANON_FORCE_INLINE Vec4d::TVec4(__m256d v)
	: v(v)
{

}

NANON_FORCE_INLINE Vec4d::TVec4(double x, double y, double z, double w)
	: v(_mm256_set_pd(w, z, y, x))
{

}

NANON_FORCE_INLINE Vec4d::TVec4(const Vec3d& v, double w)
{
	this->v = v.v;
	this->w = w;
}

NANON_FORCE_INLINE double Vec4d::operator[](int i) const
{
	return (&x)[i];
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator=(const Vec4d& v)
{
	this->v = v.v;
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator+=(const Vec4d& v)
{
	this->v = _mm256_add_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator-=(const Vec4d& v)
{
	this->v = _mm256_sub_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator*=(const Vec4d& v)
{
	this->v = _mm256_mul_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator*=(double s)
{
	this->v = _mm256_mul_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator/=(const Vec4d& v)
{
	this->v = _mm256_div_pd(this->v, v.v);
	return *this;
}

NANON_FORCE_INLINE Vec4d& Vec4d::operator/=(double s)
{
	this->v = _mm256_div_pd(this->v, _mm256_set1_pd(s));
	return *this;
}

template <>
NANON_FORCE_INLINE Vec4d operator+(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_add_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4d operator-(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_sub_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4d operator*(const Vec4d& v, const double& s)
{
	return Vec4d(_mm256_mul_pd(v.v, _mm256_set1_pd(s)));
}

template <>
NANON_FORCE_INLINE Vec4d operator*(const double& s, const Vec4d& v)
{
	return v * s;
}

template <>
NANON_FORCE_INLINE Vec4d operator*(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_mul_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4d operator/(const Vec4d& v, const double& s)
{
	return v / Vec4d(s);
}

template <>
NANON_FORCE_INLINE Vec4d operator/(const Vec4d& v1, const Vec4d& v2)
{
	return Vec4d(_mm256_div_pd(v1.v, v2.v));
}

template <>
NANON_FORCE_INLINE Vec4d operator-(const Vec4d& v)
{
	return Vec4d(_mm256_sub_pd(_mm256_setzero_pd(), v.v));
}

template <>
NANON_FORCE_INLINE double Length(const Vec4d& v)
{
	return Math::Sqrt(Length2(v));
}

template <>
NANON_FORCE_INLINE double Length2(const Vec4d& v)
{
	return Dot(v, v);
}

template <>
NANON_FORCE_INLINE Vec4d Normalize(const Vec4d& v)
{
	return v / Length(v);
}

template <>
NANON_FORCE_INLINE double Dot(const Vec4d& v1, const Vec4d& v2)
{
	__m256d t1 = _mm256_mul_pd(v1.v, v2.v);		// t1 = [ v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w ]
	__m256d t2 = _mm256_hadd_pd(t1, t1);		// t2 = [ v1.x * v2.x + v1.y * v2.y, _, v1.z * v2.z + v1.w * v2.w, _ ]
	__m128d t3 = _mm256_extractf128_pd(t2, 1);	// t3 = [ v1.x * v2.x + v1.y * v2.y, _ ]
	__m128d t4 = _mm256_castpd256_pd128(t2);	// t4 = [ v1.z * v2.z + v1.w * v2.w, _ ]
	__m128d result = _mm_add_pd(t3, t4);		// result = [ v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w, _ ]
	return Vec4d(_mm256_castpd128_pd256(result)).x;
}

#endif

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END