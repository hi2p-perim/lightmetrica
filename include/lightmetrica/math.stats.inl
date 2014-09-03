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

#include "math.basic.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T>
LM_FORCE_INLINE TVec2<T> UniformConcentricDiskSample(const TVec2<T>& u)
{
	// Uniform sampling on the square [-1, 1]^2
	T v1 = T(2) * u[0] - T(1);
	T v2 = T(2) * u[1] - T(1);

	// Convert (sx, sy) to (r, theta)
	TVec2<T> conv;

	if (v1 == T(0) && v2 == T(0))
	{
		conv = TVec2<T>(T(0));
	}
	else if (v1 > -v2)
	{
		if (v1 > v2)
		{
			conv = TVec2<T>(v1, T((TConstants<T>::Pi() / T(4)) * v2/v1));
		}
		else
		{
			conv = TVec2<T>(v2, T((TConstants<T>::Pi() / T(4)) * (T(2) - v1/v2)));
		}
	}
	else
	{
		if (v1 < v2)
		{
			conv = TVec2<T>(-v1, T((TConstants<T>::Pi() / T(4)) * (T(4) + v2/v1)));
		}
		else
		{
			conv = TVec2<T>(-v2, T((TConstants<T>::Pi() / T(4)) * (T(6) - v1/v2)));
		}
	}

	return TVec2<T>(conv.x * Cos(conv.y), conv.x * Sin(conv.y));
}

template <typename T>
LM_FORCE_INLINE TPDFEval<T> UniformConcentricDiskSamplePDF()
{
	return TPDFEval<T>(TConstants<T>::InvPi(), ProbabilityMeasure::Area);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> CosineSampleHemisphere(const TVec2<T>& u)
{
	auto s = UniformConcentricDiskSample(u);
	return TVec3<T>(s, Sqrt(Max<T>(T(0), T(1) - s.x*s.x - s.y*s.y)));
}

template <typename T>
LM_FORCE_INLINE TPDFEval<T> CosineSampleHemispherePDF(const TVec3<T>& d)
{
	return TPDFEval<T>(TConstants<T>::InvPi() * CosThetaZUp(d), ProbabilityMeasure::SolidAngle);
}

template <typename T>
LM_FORCE_INLINE TPDFEval<T> CosineSampleHemispherePDFProjSA(const TVec3<T>& d)
{
	return TPDFEval<T>(TConstants<T>::InvPi(), ProbabilityMeasure::ProjectedSolidAngle);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> UniformSampleHemisphere(const TVec2<T>& u)
{
	const T& z = u[0];
	T r = Sqrt(Max<T>(T(0), T(1) - z*z));
	T phi = T(2) * TConstants<T>::Pi() * u[1];
	return TVec3<T>(r * Cos(phi), r * Sin(phi), z);
}

template <typename T>
LM_FORCE_INLINE TPDFEval<T> UniformSampleHemispherePDF()
{
	return TPDFEval<T>(TConstants<T>::InvTwoPi(), ProbabilityMeasure::SolidAngle);
}

template <typename T>
LM_FORCE_INLINE TVec3<T> UniformSampleSphere(const TVec2<T>& u)
{
	T z = T(1) - T(2) * u[0];
	T r = Sqrt(Max(T(0), T(1) - z*z));
	T phi = T(2) * TConstants<T>::Pi() * u[1];
	return TVec3<T>(r * Cos(phi), r * Sin(phi), z);
}

template <typename T>
LM_FORCE_INLINE TPDFEval<T> UniformSampleSpherePDF()
{
	return TPDFEval<T>(TConstants<T>::InvTwoPi() / T(2), ProbabilityMeasure::SolidAngle);
}

template <typename T>
LM_FORCE_INLINE TVec2<T> UniformSampleTriangle(const TVec2<T>& u)
{
	T s = Sqrt(Max(T(0), u[0]));
	return TVec2<T>(T(1) - s, u[1] * s);
}

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END