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
#ifndef LIB_LIGHTMETRICA_FP_H
#define LIB_LIGHTMETRICA_FP_H

#include "common.h"

LM_NAMESPACE_BEGIN

/*!
	Floating-point utilities.
	Contains functions on floating-point exception control.
*/
class LM_PUBLIC_API FloatintPointUtils
{
private:

	FloatintPointUtils();
	~FloatintPointUtils();

	LM_DISABLE_COPY_AND_MOVE(FloatintPointUtils);

public:

	/*!
		Enable floating point control.
		This function is available only for strict floating-point mode and in Windows.
	*/
	static bool EnableFPControl();

	/*!
		Disable floating point control.
		This function is available only for strict floating-point mode and in Windows.
	*/
	static bool DisableFPControl();

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_FP_H