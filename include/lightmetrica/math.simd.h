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
#ifndef LIB_LIGHTMETRICA_MATH_SIMD_H
#define LIB_LIGHTMETRICA_MATH_SIMD_H

#include "simdsupport.h"

#if LM_SSE2
#include <xmmintrin.h>
#endif
#if LM_SSE3
#include <pmmintrin.h>
#endif
#if LM_SSSE3
#include <tmmintrin.h>
#endif
#if LM_SSE4_1
#include <smmintrin.h>
#endif
#if LM_SSE4_2
#include <nmmintrin.h>
#endif
#if LM_SSE4A
#include <ammintrin.h>
#endif
#if LM_AVX
#include <immintrin.h>
#endif

#endif // LIB_LIGHTMETRICA_MATH_SIMD_H