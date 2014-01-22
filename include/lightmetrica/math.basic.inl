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

#include "math.basic.h"
#include "math.constants.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename T> LM_FORCE_INLINE T Radians(const T& v) { return v * TConstants<T>::Pi() / T(180); }
template <typename T> LM_FORCE_INLINE T Degrees(const T& v) { return v * T(180) / TConstants<T>::Pi(); }
template <typename T> LM_FORCE_INLINE T Cos(const T& v) { return std::cos(v); }
template <typename T> LM_FORCE_INLINE T Sin(const T& v) { return std::sin(v); }
template <typename T> LM_FORCE_INLINE T Tan(const T& v) { return std::tan(v); }
template <typename T> LM_FORCE_INLINE T Abs(const T& v) { return std::abs(v); }
template <typename T> LM_FORCE_INLINE T Sqrt(const T& v) { return std::sqrt(v); }
template <typename T> LM_FORCE_INLINE T Min(const T& v1, const T& v2) { return std::min(v1, v2); }
template <typename T> LM_FORCE_INLINE T Max(const T& v1, const T& v2) { return std::max(v1, v2); }
template <typename T> LM_FORCE_INLINE T Clamp(const T& v, const T& min, const T& max) { return Min(Max(v, min), max); }

#ifdef LM_ENABLE_MULTI_PRECISION

template <> LM_FORCE_INLINE BigFloat Cos(const BigFloat& v) { return boost::multiprecision::cos(v); }
template <> LM_FORCE_INLINE BigFloat Sin(const BigFloat& v) { return boost::multiprecision::sin(v); }
template <> LM_FORCE_INLINE BigFloat Tan(const BigFloat& v) { return boost::multiprecision::tan(v); }
template <> LM_FORCE_INLINE BigFloat Abs(const BigFloat& v) { return boost::multiprecision::abs(v); }
template <> LM_FORCE_INLINE BigFloat Sqrt(const BigFloat& v) { return boost::multiprecision::sqrt(v); }
template <> LM_FORCE_INLINE BigFloat Min(const BigFloat& v1, const BigFloat& v2) { return v1 > v2 ? v2 : v1; }
template <> LM_FORCE_INLINE BigFloat Max(const BigFloat& v1, const BigFloat& v2) { return v1 > v2 ? v1 : v2; }

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END
