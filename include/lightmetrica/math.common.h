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
#if LM_SINGLE_PRECISION + LM_DOUBLE_PRECISION + LM_DOUBLE_PRECISION != 1
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