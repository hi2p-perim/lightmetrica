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

#ifndef __LIB_NANON_MATH_TYPES_H__
#define __LIB_NANON_MATH_TYPES_H__

#include "math.common.h"
#include "simdsupport.h"

// Generic implementation of math functions
// if there is no SIMD support the implementation is used instead.
#include "math.vector.h"
#include "math.matrix.h"
#include "math.quat.h"

// Specialized implementation optimized by SIMD instructions
#ifndef NANON_FORCE_NO_SIMD
	#ifdef NANON_USE_SSE2
		#include "math.ssevector.h"
		#include "math.ssematrix.h"
		#include "math.ssequat.h"
	#endif
	#ifdef NANON_USE_AVX
		#include "math.avxvector.h"
		#include "math.avxmatrix.h"
		#include "math.avxquat.h"
	#endif
#endif

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

// Define default floating point types
#ifdef NANON_SINGLE_PRECISION
	typedef float Float;
	typedef Vec2f Vec2;
	typedef Vec3f Vec3;
	typedef Vec4f Vec4;
	typedef Mat3f Mat3;
	typedef Mat4f Mat4;
#elif defined(NANON_DOUBLE_PRECISION)
	typedef double Float;
	typedef Vec2d Vec2;
	typedef Vec3d Vec3;
	typedef Vec4d Vec4;
	typedef Mat3d Mat3;
	typedef Mat4d Mat4;
#elif defined(NANON_MULTI_PRECISION)
	#include <boost/multiprecision/cpp_dec_float.hpp>
	#ifndef NANON_PRECISION_NUM
		// Default precision : 100 decimal digits
		#define NANON_PRECISION_NUM 100
	#endif
	typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<NANON_PPRECISION_NUM>> Float;
	typedef Vec2<Float> Vec2;
	typedef Vec3<Float> Vec3;
	typedef Vec4<Float> Vec4;
	typedef Mat3<Float> Mat3;
	typedef Mat4<Float> Mat4;
#endif

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#endif // __LIB_NANON_MATH_TYPES_H__