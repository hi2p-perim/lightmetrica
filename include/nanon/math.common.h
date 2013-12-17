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

#ifndef __LIB_NANON_MATH_COMMON_H__
#define __LIB_NANON_MATH_COMMON_H__

#include "common.h"
#include <cmath>

#define NANON_MATH_NAMESPACE_BEGIN namespace Math {
#define NANON_MATH_NAMESPACE_END }

// Multiprecision support
// Note that the multiprecision support can be enabled irrelevant to 
// the macro NANON_MULTI_PRECISION for testing.
#ifdef NANON_ENABLE_MULTIPRECISION
	#include <boost/multiprecision/cpp_dec_float.hpp>
	#ifndef NANON_PRECISION_NUM
		// Default precision : 50 decimal digits
		#define NANON_PRECISION_NUM 50
	#endif
	NANON_NAMESPACE_BEGIN
	NANON_MATH_NAMESPACE_BEGIN
		#ifdef NANON_ENABLE_MULTIPRECISION
			typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<NANON_PRECISION_NUM>> BigFloat;
		#endif
	NANON_MATH_NAMESPACE_END
	NANON_NAMESPACE_END
#endif

#endif // __LIB_NANON_MATH_COMMON_H__