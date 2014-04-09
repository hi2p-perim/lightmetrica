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
#ifndef LIB_LIGHTMETRICA_MATH_TYPES_H
#define LIB_LIGHTMETRICA_MATH_TYPES_H

#include "math.vector.h"
#include "math.matrix.h"
#include "math.quat.h"
#include "math.constants.h"
#include "math.colors.h"
#include "math.cast.h"
#include "math.pdf.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

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

// Define default floating point types
#if LM_SINGLE_PRECISION
	typedef float Float;
#elif LM_DOUBLE_PRECISION
	typedef double Float;
#elif LM_MULTI_PRECISION
	#ifndef LM_ENABLE_MULTI_PRECISION
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
typedef TPDFEval<Float> PDFEval;

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_MATH_TYPES_H