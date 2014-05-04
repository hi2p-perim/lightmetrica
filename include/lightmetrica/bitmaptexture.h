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