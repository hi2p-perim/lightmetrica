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
#ifndef LIB_LIGHTMETRICA_MATH_CONSTANTS_H
#define LIB_LIGHTMETRICA_MATH_CONSTANTS_H

#include "math.common.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

/*!
	Math constants.
	\tparam T Internal value type.
*/
template <typename T>
class TConstants
{
public:

	TConstants();
	~TConstants();

	LM_DISABLE_COPY_AND_MOVE(TConstants);

public:

	LM_FORCE_INLINE static T Zero();
	LM_FORCE_INLINE static T Pi();
	LM_FORCE_INLINE static T InvPi();
	LM_FORCE_INLINE static T InvTwoPi();
	LM_FORCE_INLINE static T Inf();
	LM_FORCE_INLINE static T Eps();
	LM_FORCE_INLINE static T EpsLarge();
	LM_FORCE_INLINE static T MachineEps();

};

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#include "math.constants.inl"

#endif // LIB_LIGHTMETRICA_MATH_CONSTANTS_H
