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
#ifndef LIB_LIGHTMETRICA_BITMAP_H
#define LIB_LIGHTMETRICA_BITMAP_H

#include "common.h"
#include "math.types.h"
#include <vector>

LM_NAMESPACE_BEGIN

/*!
	Bitmap image.
	Interface for the classes which uses bitmap image.
*/
class BitmapImage
{
public:

	BitmapImage() {}
	virtual ~BitmapImage() {}

public:

	/*!
		Clear internal data.
		Clear internal data to empty.
	*/
	LM_PUBLIC_API void Clear();

	/*!
		Get reference to the internal data.
		\return Reference to the internal data.
	*/
	LM_PUBLIC_API std::vector<Math::Float>& InternalData();

	/*!
		Get reference to the internal data.
		\return Reference to the internal data.
	*/
	LM_PUBLIC_API const std::vector<Math::Float>& InternalData() const;

	/*!
		Evaluate RMSE.
		Evaluate root mean square error (RMSE) to the given #film.
		The other film must be same size and type.
		\param bitmap Other bitmap image.
		\return Evaluated RMSE.
	*/
	LM_PUBLIC_API Math::Float EvaluateRMSE(const BitmapImage& bitmap) const;

	/*!
		Evaluate RMSE with weight.
		Evaluate root mean square error (RMSE) to the given #film.
		The other film must be same size and type and scaled by #weight for each pixel value.
		\param bitmap Other bitmap image.
		\param weight Per-pixel weight.
		\return Evaluated RMSE.
	*/
	LM_PUBLIC_API Math::Float EvaluateRMSE(const BitmapImage& bitmap, const Math::Float& weight) const;

private:

	std::vector<Math::Float> data;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BITMAP_H