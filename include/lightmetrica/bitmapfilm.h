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
#ifndef LIB_LIGHTMETRICA_BITMAP_FILM_H
#define LIB_LIGHTMETRICA_BITMAP_FILM_H

#include "film.h"

LM_NAMESPACE_BEGIN

/*!
	Bitmap image types.
	Defines output bitmap image types.
*/
enum class BitmapImageType
{
	RadianceHDR,	//!< Radiance HDR
	OpenEXR			//!< OpenEXR
};

class BitmapImage;

/*!
	Bitmap film.
	An interface for films contained in a bitmap image.
*/
class BitmapFilm : public Film
{
public:

	BitmapFilm() {}
	virtual ~BitmapFilm() {}

public:

	/*!
		Save as image.
		Saves the film as image.
		If #path is empty, the default path is used.
		\param path Path to the output image.
		\retval true Succeeded to save the image.
		\retval false Failed to save the image.
	*/
	virtual bool Save(const std::string& path) const = 0;

	/*!
		Rescale and save as image.
		Save the film as image after rescaling values for each pixel.
		\param path Path to the output image.
		\param weight Rescaling weight.
		\retval true Succeeded to save the image.
		\retval false Failed to save the image.
	*/
	virtual bool RescaleAndSave(const std::string& path, const Math::Float& weight) const = 0;

	/*!
		Allocate the film with given width and height.
		\param width Width of the film.
		\param height Height of the film.
	*/
	virtual void Allocate(int width, int height) = 0;

	/*!
		Set bitmap image type.
		\param type Bitmap image type.
	*/
	virtual void SetImageType(BitmapImageType type) = 0;

	/*!
		Get bitmap image type.
		\return Bitmap image type.
	*/
	virtual BitmapImageType ImageType() const = 0;

	/*!
		Get internal bitmap data.
		\return A bitmap.
	*/
	virtual const BitmapImage& Bitmap() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BITMAP_FILM_H