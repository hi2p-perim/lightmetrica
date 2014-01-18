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

#pragma once
#ifndef __LIB_LIGHTMETRICA_MATH_STATS_H__
#define __LIB_LIGHTMETRICA_MATH_STATS_H__

#include "math.vector.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> LM_FORCE_INLINE TVec2<T> ConcentricDiskSample(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TVec3<T> CosineSampleHemisphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TVec3<T> UniformSampleHemisphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TVec3<T> UniformSampleSphere(const TVec2<T>& u);
template <typename T> LM_FORCE_INLINE TVec2<T> UniformSampleTriangle(const TVec2<T>& u);

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.stats.inl"

#endif // __LIB_LIGHTMETRICA_MATH_STATS_H__