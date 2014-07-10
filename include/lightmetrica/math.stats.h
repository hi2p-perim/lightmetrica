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

#pragma once
#ifndef LIB_LIGHTMETRICA_MATH_STATS_H
#define LIB_LIGHTMETRICA_MATH_STATS_H

#include "math.vector.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> LM_FORCE_INLINE TVec2<T> UniformConcentricDiskSample(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TPDFEval<T> UniformConcentricDiskSamplePDF();
template <typename T> LM_FORCE_INLINE TVec3<T> CosineSampleHemisphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TPDFEval<T> CosineSampleHemispherePDF(const TVec3<T>& d);
template <typename T> LM_FORCE_INLINE TPDFEval<T> CosineSampleHemispherePDFProjSA(const TVec3<T>& d);
template <typename T> LM_FORCE_INLINE TVec3<T> UniformSampleHemisphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TPDFEval<T> UniformSampleHemispherePDF();
template <typename T> LM_FORCE_INLINE TVec3<T> UniformSampleSphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TPDFEval<T> UniformSampleSphere();
template <typename T> LM_FORCE_INLINE TVec2<T> UniformSampleTriangle(const TVec2<T>& u);

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.stats.inl"

#endif // LIB_LIGHTMETRICA_MATH_STATS_H