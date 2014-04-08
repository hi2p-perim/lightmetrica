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

private:

	std::vector<Math::Float> data;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BITMAP_H