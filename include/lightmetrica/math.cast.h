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
#ifndef LIB_LIGHTMETRICA_MATH_CAST_H
#define LIB_LIGHTMETRICA_MATH_CAST_H

#include "math.common.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

template <typename Dest, typename Src>
LM_FORCE_INLINE Dest Cast(const Src& v)
{
	// In the default, use static_cast operator
	return static_cast<Dest>(v);
}

#ifdef LM_ENABLE_MULTI_PRECISION

template <typename Dest>
LM_FORCE_INLINE Dest Cast(const BigFloat& v)
{
	// For BigFloat type, use convert_to function
	return v.convert_to<Dest>();
}

#endif

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_MATH_CAST_H