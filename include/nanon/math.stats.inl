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

#include "math.basic.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

template <typename T>
NANON_FORCE_INLINE TVec2<T> ConcentricDiskSample(const TVec2<T>& u)
{
	// Uniform sampling on the square [-1, 1]^2
	T v1 = T(2) * u[0] - T(1);
	T v2 = T(2) * u[1] - T(1);

	// Convert (sx, sy) to (r, theta)
	Math::Vec2 conv;

	if (v1 == T(0) && v2 == T(0))
	{
		conv = Math::Vec2(T(0));
	}
	else if (v1 > -v2)
	{
		if (v1 > v2)
		{
			conv = Math::Vec2(v1, T((Math::Constants::Pi / T(4)) * v2/v1));
		}
		else
		{
			conv = Math::Vec2(v2, T((Math::Constants::Pi / T(4)) * (T(2) - v1/v2)));
		}
	}
	else
	{
		if (v1 < v2)
		{
			conv = Math::Vec2(-v1, T((Math::Constants::Pi / T(4)) * (T(4) + v2/v1)));
		}
		else
		{
			conv = Math::Vec2(-v2, T((Math::Constants::Pi / T(4)) * (T(6) - v1/v2)));
		}
	}

	return Math::Vec2(conv.x * Math::Cos(conv.y), conv.x * Math::Sin(conv.y));
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> CosineSampleHemisphere(const TVec2<T>& u)
{
	auto s = ConcentricDiskSample(u);
	return Math::Vec3(s, Math::Sqrt(Math::Max(T(0), T(1) - s.x*s.x - s.y*s.y)));
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> UniformSampleHemisphere(const TVec2<T>& u)
{
	const T& z = u[0];
	T r = Math::Sqrt(Math::Max(0.0, 1.0 - z*z));
	T phi = 2.0 * Pi * u[1];
	return Math::Vec3(r * Math::Cos(phi), r * Math::Sin(phi), z);
}

template <typename T>
NANON_FORCE_INLINE TVec3<T> UniformSampleSphere(const TVec2<T>& u)
{
	T z = 1.0 - 2.0 * u[0];
	T r = Math::Sqrt(Math::Max(T(0), T(1) - z*z));
	T phi = T(2) * Pi * u[1];
	return Math::Vec3(r * Math::Cos(phi), r * Math::Sin(phi), z);
}

template <typename T>
NANON_FORCE_INLINE TVec2<T> UniformSampleTriangle(const TVec2<T>& u)
{
	T s = Math::Sqrt(Math::Max(T(0), u[0]));
	return Math::Vec2(T(1) - s, u[1] * s);
}

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END