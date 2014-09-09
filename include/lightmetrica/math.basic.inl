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
template <typename T> LM_FORCE_INLINE T Log(const T& v) { return std::log(v); }
template <typename T> LM_FORCE_INLINE T Exp(const T& v) { return std::exp(v); }
template <typename T> LM_FORCE_INLINE T Ceil(const T& v) { return std::ceil(v); }
template <typename T> LM_FORCE_INLINE T Pow(const T& base, const T& exp) { return std::pow(base, exp); }
template <typename T> LM_FORCE_INLINE T Min(const T& v1, const T& v2) { return std::min(v1, v2); }
template <typename T> LM_FORCE_INLINE T Max(const T& v1, const T& v2) { return std::max(v1, v2); }
template <typename T> LM_FORCE_INLINE T Clamp(const T& v, const T& min, const T& max) { return Min(Max(v, min), max); }
template <typename T> LM_FORCE_INLINE T Fract(const T& v) { return v - std::floor(v); }
template <typename T> LM_FORCE_INLINE bool IsZero(const T& v) { return v == T(0); }

#ifdef LM_ENABLE_MULTI_PRECISION

template <> LM_FORCE_INLINE BigFloat Cos(const BigFloat& v) { return boost::multiprecision::cos(v); }
template <> LM_FORCE_INLINE BigFloat Sin(const BigFloat& v) { return boost::multiprecision::sin(v); }
template <> LM_FORCE_INLINE BigFloat Tan(const BigFloat& v) { return boost::multiprecision::tan(v); }
template <> LM_FORCE_INLINE BigFloat Abs(const BigFloat& v) { return boost::multiprecision::abs(v); }
template <> LM_FORCE_INLINE BigFloat Sqrt(const BigFloat& v) { return boost::multiprecision::sqrt(v); }
template <> LM_FORCE_INLINE BigFloat Log(const BigFloat& v) { return boost::multiprecision::log(v); }
template <> LM_FORCE_INLINE BigFloat Exp(const BigFloat& v) { return boost::multiprecision::exp(v); }
template <> LM_FORCE_INLINE BigFloat Ceil(const BigFloat& v) { return boost::multiprecision::ceil(v); }
template <> LM_FORCE_INLINE BigFloat Pow(const BigFloat& base, const BigFloat& exp) { return boost::multiprecision::pow(base, exp); };
template <> LM_FORCE_INLINE BigFloat Min(const BigFloat& v1, const BigFloat& v2) { return v1 > v2 ? v2 : v1; }
template <> LM_FORCE_INLINE BigFloat Max(const BigFloat& v1, const BigFloat& v2) { return v1 > v2 ? v1 : v2; }
template <> LM_FORCE_INLINE BigFloat Fract(const BigFloat& v) { return v - boost::multiprecision::floor(v); }

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END
