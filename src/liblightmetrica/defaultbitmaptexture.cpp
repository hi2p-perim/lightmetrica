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

#include "pch.h"
#include <lightmetrica/bitmaptexture.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/pathutils.h>
#include <lightmetrica/bitmap.h>
#include <FreeImage.h>

LM_NAMESPACE_BEGIN

/*!
	Bitmap texture.
	Implements a bitmap texture.
*/
class DefaultBitmapTexture : public BitmapTexture
{
public:

	LM_COMPONENT_IMPL_DEF("bitmap");

public:

	DefaultBitmapTexture() {}
	virtual ~DefaultBitmapTexture() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool Load(const std::string& path, bool verticalFlip);
	virtual const BitmapImage& Bitmap() const { return bitmap; }

public:

	static void FreeImageErrorCallback(FREE_IMAGE_FORMAT fif, const char* message);

private:

	DefaultBitmapTexture* self;
	int width;
	int height;
	BitmapImage bitmap;

};

bool DefaultBitmapTexture::Load( const ConfigNode& node, const Assets& assets )
{
	// 'path' element
	std::string path;
	if (!node.ChildValue("path", path))
	{
		return false;
	}

	// Resolve the path
	path = PathUtils::ResolveAssetPath(*node.GetConfig(), path);

	// 'vertical_flip' element
	bool verticalFlip;
	node.ChildValueOrDefault("vertical_flip", false, verticalFlip);

	return Load(path, verticalFlip);
}

bool DefaultBitmapTexture::Load( const std::string& path, bool verticalFlip )
{
	// Try to deduce the file format by the file signature
	auto format = FreeImage_GetFileType(path.c_str(), 0);
	if (format == FIF_UNKNOWN)
	{
		// Try to deduce the file format by the extension
		format = FreeImage_GetFIFFromFilename(path.c_str());
		if (format == FIF_UNKNOWN)
		{
			// Unknown image
			LM_LOG_ERROR("Unknown image format");
			return false;
		}
	}

	// Check the plugin capability
	if (!FreeImage_FIFSupportsReading(format))
	{
		LM_LOG_ERROR("Unsupported format");
		return false;
	}

	// Load image
	auto* fibitmap = FreeImage_Load(format, path.c_str(), 0);
	if (!fibitmap)
	{
		LM_LOG_ERROR("Failed to load an image " + path);
		return false;
	}

	// Width and height
	width	= FreeImage_GetWidth(fibitmap);
	height	= FreeImage_GetHeight(fibitmap);

	// Image type and bits per pixel (BPP)
	auto type	= FreeImage_GetImageType(fibitmap);
	auto bpp	= FreeImage_GetBPP(fibitmap);
	if (!(type == FIT_RGBF || type == FIT_RGBAF || (type == FIT_BITMAP && (bpp == 24 || bpp == 32))))
	{
		FreeImage_Unload(fibitmap);
		LM_LOG_ERROR("Unsupportted format");
		return false;
	}

	// Flip the loaded image
	// Note that in FreeImage loaded image is flipped from the beginning,
	// i.e., y axis is originated from bottom-left point and grows upwards.
	if (!verticalFlip)
	{
		FreeImage_FlipVertical(fibitmap);
	}

	// Read image data
	auto& data = bitmap.InternalData();
	data.clear();
	for (int y = 0; y < height; y++)
	{
		if (type == FIT_RGBF)
		{
			auto* bits = (FIRGBF*)FreeImage_GetScanLine(fibitmap, y);
			for (int x = 0; x < width; x++)
			{
				data.emplace_back(bits[x].red);
				data.emplace_back(bits[x].green);
				data.emplace_back(bits[x].blue);
			}
		}
		else if (type == FIT_RGBAF)
		{
			auto* bits = (FIRGBAF*)FreeImage_GetScanLine(fibitmap, y);
			for (int x = 0; x < width; x++)
			{
				data.emplace_back(bits[x].red);
				data.emplace_back(bits[x].green);
				data.emplace_back(bits[x].blue);
			}
		}
		else if (type == FIT_BITMAP)
		{
			BYTE* bits = (BYTE*)FreeImage_GetScanLine(fibitmap, y);
			for (int x = 0; x < width; x++)
			{
				data.push_back(Math::Float(bits[FI_RGBA_RED  ]) / Math::Float(255));
				data.push_back(Math::Float(bits[FI_RGBA_GREEN]) / Math::Float(255));
				data.push_back(Math::Float(bits[FI_RGBA_BLUE ]) / Math::Float(255));
				bits += bpp / 8;
			}
		}
	}

	FreeImage_Unload(fibitmap);

	return true;
}

LM_COMPONENT_REGISTER_IMPL(DefaultBitmapTexture, Texture);

LM_NAMESPACE_END