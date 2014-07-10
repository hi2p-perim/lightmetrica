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
#ifndef LIB_LIGHTMETRICA_MATH_TYPES_H
#define LIB_LIGHTMETRICA_MATH_TYPES_H

#include "math.common.h"
#include "math.vector.h"
#include "math.matrix.h"
#include "math.quat.h"
#include "math.constants.h"
#include "math.colors.h"
#include "math.cast.h"
#include "math.pdf.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

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