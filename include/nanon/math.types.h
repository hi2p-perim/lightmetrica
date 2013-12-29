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

#pragma once
#ifndef __LIB_NANON_MATH_TYPES_H__
#define __LIB_NANON_MATH_TYPES_H__

#include "math.vector.h"
#include "math.matrix.h"
#include "math.quat.h"
#include "math.constants.h"
#include "math.colors.h"

NANON_NAMESPACE_BEGIN
NANON_MATH_NAMESPACE_BEGIN

// Define default floating point types
#ifdef NANON_SINGLE_PRECISION
	typedef float Float;
#elif defined(NANON_DOUBLE_PRECISION)
	typedef double Float;
#elif defined(NANON_MULTI_PRECISION)
	#ifndef NANON_ENABLE_MULTI_PRECISION
		#error "Multiprecision support must be enabled"
	#endif
	typedef BigFloat Float;
#endif

typedef TVec2<Float> Vec2;
typedef TVec3<Float> Vec3;
typedef TVec4<Float> Vec4;
typedef TMat3<Float> Mat3;
typedef TMat4<Float> Mat4;
typedef TConstants<Float> Constants;
typedef TColors<Float> Colors;

NANON_MATH_NAMESPACE_END
NANON_NAMESPACE_END

#endif // __LIB_NANON_MATH_TYPES_H__