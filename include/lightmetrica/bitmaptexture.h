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
#ifndef LIB_LIGHTMETRICA_BITMAP_TEXTURE_H
#define LIB_LIGHTMETRICA_BITMAP_TEXTURE_H

#include "texture.h"

LM_NAMESPACE_BEGIN

class BitmapImage;

/*!
	Bitmap texture.
	An interface for textures contained in a bitmap texture.
*/
class BitmapTexture : public Texture
{
public:

	BitmapTexture() {}
	virtual ~BitmapTexture() {}

public:

	// For overloading #Load function
	using Asset::Load;

	/*
		Load a bitmap texture.
		\param path Path to a texture.
		\param verticalFlip If true, loaded image is flipped vertically.
		\retval true Succeeded to load.
		\retval false Failed to load.
	*/
	virtual bool Load(const std::string& path, bool verticalFlip = false) = 0;

	/*!
		Get internal bitmap data.
		\return A bitmap.
	*/
	virtual const BitmapImage& Bitmap() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BITMAP_TEXTURE_H