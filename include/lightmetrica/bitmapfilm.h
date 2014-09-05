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
	OpenEXR,		//!< OpenEXR
	PNG,
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