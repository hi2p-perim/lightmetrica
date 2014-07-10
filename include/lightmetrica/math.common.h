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
#ifndef LIB_LIGHTMETRICA_MATH_COMMON_H
#define LIB_LIGHTMETRICA_MATH_COMMON_H

#include "common.h"
#include <cmath>

#define LM_MATH_NAMESPACE_BEGIN namespace Math {
#define LM_MATH_NAMESPACE_END }

// Precision mode
#ifdef LM_USE_SINGLE_PRECISION
	#define LM_SINGLE_PRECISION 1
#else
	#define LM_SINGLE_PRECISION 0
#endif
#ifdef LM_USE_DOUBLE_PRECISION
	#define LM_DOUBLE_PRECISION 1
#else
	#define LM_DOUBLE_PRECISION 0
#endif
#ifdef LM_USE_MULTI_PRECISION
	#define LM_MULTI_PRECISION 1
#else
	#define LM_MULTI_PRECISION 0
#endif
#if LM_SINGLE_PRECISION + LM_DOUBLE_PRECISION + LM_MULTI_PRECISION != 1
	#error "Invalid precision mode"
#endif

// Multiprecision support
// Note that the multiprecision support can be enabled irrelevant to 
// the macro LM_MULTI_PRECISION for testing.
#ifdef LM_ENABLE_MULTI_PRECISION
	#include <boost/multiprecision/cpp_dec_float.hpp>
	#ifndef LM_PRECISION_NUM
		// Default precision : 50 decimal digits
		#define LM_PRECISION_NUM 50
	#endif
	LM_NAMESPACE_BEGIN
	LM_MATH_NAMESPACE_BEGIN
		#ifdef LM_ENABLE_MULTI_PRECISION
			typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<LM_PRECISION_NUM>> BigFloat;
		#endif
	LM_MATH_NAMESPACE_END
	LM_NAMESPACE_END
#endif

#endif // LIB_LIGHTMETRICA_MATH_COMMON_H