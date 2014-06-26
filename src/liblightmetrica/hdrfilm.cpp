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
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/bitmap.h>
#include <FreeImage.h>

LM_NAMESPACE_BEGIN

/*!
	High dynamic range bitmap film.
	Implements HDR version of bitmap image recording.
*/
class HDRBitmapFilm : public BitmapFilm
{
public:

	LM_COMPONENT_IMPL_DEF("hdr");

public:

	HDRBitmapFilm() {}
	virtual ~HDRBitmapFilm() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );
	
public:

	virtual int Width() const { return width; }
	virtual int Height() const { return height; }
	virtual void RecordContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb);
	virtual void AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb );
	virtual void AccumulateContribution( const Film& film );
	virtual void Rescale( const Math::Float& weight );
	virtual Film* Clone() const;

public:

	virtual bool Save(const std::string& path) const { return RescaleAndSave(path, Math::Float(1)); }
	virtual bool RescaleAndSave( const std::string& path, const Math::Float& weight ) const;
	virtual void Allocate(int width, int height);
	virtual void SetImageType(BitmapImageType type) { this->type = type; }
	virtual BitmapImageType ImageType() const { return type; }
	virtual const BitmapImage& Bitmap() const { return bitmap; }

public:

	static void FreeImageErrorCallback(FREE_IMAGE_FORMAT fif, const char* message);

private:

	int width;
	int height;
	BitmapImageType type;		// Type of the image to be saved
	BitmapImage bitmap;

};

bool HDRBitmapFilm::Load( const ConfigNode& node, const Assets& assets )
{
	if (!node.ChildValue("width",	width))		return false;
	if (!node.ChildValue("height",	height))	return false;

	// Find 'image_type' element (optional)
	auto imageTypeNode = node.Child("imagetype");
	if (imageTypeNode.Empty())
	{
		// Use .hdr as default type
		SetImageType(BitmapImageType::RadianceHDR);
	}
	else
	{
		if (imageTypeNode.Value() == "radiancehdr")
		{
			// Image type is .hdr (Radiance HDR)
			SetImageType(BitmapImageType::RadianceHDR);
		}
		else if (imageTypeNode.Value() == "openexr")
		{
			// Image type is .exr (OpenEXR)
			SetImageType(BitmapImageType::OpenEXR);
		}
		else
		{
			LM_LOG_ERROR("Invalid image type '" + imageTypeNode.Value() + "'");
			return false;
		}
	}

	// Allocate image data
	Allocate(width, height);

	return true;
}

void HDRBitmapFilm::FreeImageErrorCallback( FREE_IMAGE_FORMAT fif, const char* message )
{
	std::string format = fif != FIF_UNKNOWN
		? FreeImage_GetFormatFromFIF(fif)
		: "unknown";

	LM_LOG_ERROR(format);
	LM_LOG_ERROR(message);
}

void HDRBitmapFilm::RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	// Check raster position
	if (rasterPos.x < 0 || 1 < rasterPos.x || rasterPos.y < 0 || 1 < rasterPos.y)
	{
		LM_LOG_WARN(boost::str(boost::format("Invalid raster position (%d, %d)") % rasterPos.x % rasterPos.y));
		return;
	}

	// Convert raster position to pixel position
	Math::Vec2i pixelPos(
		Math::Clamp(Math::Cast<int>(Math::Float(rasterPos.x * Math::Float(width))), 0, width-1),
		Math::Clamp(Math::Cast<int>(Math::Float(rasterPos.y * Math::Float(height))), 0, height-1));

	// Record contribution
	size_t idx = pixelPos.y * width + pixelPos.x;
	auto& data = bitmap.InternalData();
	data[3 * idx    ] = contrb[0];
	data[3 * idx + 1] = contrb[1];
	data[3 * idx + 2] = contrb[2];
}

void HDRBitmapFilm::AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	// Check raster position
	if (rasterPos.x < 0 || 1 < rasterPos.x || rasterPos.y < 0 || 1 < rasterPos.y)
	{
		LM_LOG_WARN(boost::str(boost::format("Invalid raster position (%d, %d)") % rasterPos.x % rasterPos.y));
		return;
	}

	// Convert raster position to pixel position
	Math::Vec2i pixelPos(
		Math::Clamp(Math::Cast<int>(Math::Float(rasterPos.x * Math::Float(width))), 0, width-1),
		Math::Clamp(Math::Cast<int>(Math::Float(rasterPos.y * Math::Float(height))), 0, height-1));

	// Accumulate contribution
	size_t idx = pixelPos.y * width + pixelPos.x;
	auto& data = bitmap.InternalData();
	data[3 * idx    ] += contrb[0];
	data[3 * idx + 1] += contrb[1];
	data[3 * idx + 2] += contrb[2];
}

void HDRBitmapFilm::AccumulateContribution( const Film& film )
{
	// Check type
	if (film.ComponentImplTypeName() != ComponentImplTypeName())
	{
		LM_LOG_WARN("Invalid image type '" + film.ComponentImplTypeName() + "', expected '" + ComponentInterfaceTypeName() + "'");
		return;
	}

	// Check image size
	if (film.Width() != width || film.Height() != height)
	{
		LM_LOG_WARN("Invalid image size");
		return;
	}

	// Accumulate data
	const auto& otherData = dynamic_cast<const HDRBitmapFilm&>(film).bitmap.InternalData();
	auto& data = bitmap.InternalData();
	LM_ASSERT(data.size() == otherData.size());
	for (size_t i = 0; i < data.size(); i++)
	{
		data[i] += otherData[i];
	}
}

bool HDRBitmapFilm::RescaleAndSave( const std::string& path, const Math::Float& weight ) const
{
	// Error handing of FreeImage
	FreeImage_SetOutputMessage(FreeImageErrorCallback);

	// If #path is empty, the default path is used
	std::string imagePath = path;
	if (imagePath.empty())
	{
		imagePath = "result.hdr";
		LM_LOG_WARN("Output image path is not specified. Using '" + imagePath + "' as default.");
	}

	// Create bitmap
	// 128 bit RGBA float image
	// Note: EXR - FIT_RGBAF, HDR - FIT_RGBF
	FIBITMAP* fibitmap = FreeImage_AllocateT(FIT_RGBF, width, height);
	if (!fibitmap)
	{
		LM_LOG_ERROR("Failed to allocate bitmap");
		return false;
	}

	// Copy data
	auto& data = bitmap.InternalData();
	for (int y = 0; y < height; y++)
	{
		FIRGBF* bits = (FIRGBF*)FreeImage_GetScanLine(fibitmap, y);
		for (int x = 0; x < width; x++)
		{
			int idx = y * width + x;
			bits[x].red		= Math::Cast<float>(data[3 * idx    ] * weight);
			bits[x].green	= Math::Cast<float>(data[3 * idx + 1] * weight);
			bits[x].blue	= Math::Cast<float>(data[3 * idx + 2] * weight);
		}
	}

	BOOL result = false;
	if (type == BitmapImageType::RadianceHDR)
	{
		// Save image as Radiance HDR format
		result = FreeImage_Save(FIF_HDR, fibitmap, imagePath.c_str(), HDR_DEFAULT);
	}
	else if (type == BitmapImageType::OpenEXR)
	{
		// Save image as OpenEXR format
		result = FreeImage_Save(FIF_EXR, fibitmap, imagePath.c_str(), EXR_DEFAULT);
	}

	if (!result)
	{
		LM_LOG_DEBUG("Failed to save image : " + imagePath);
		FreeImage_Unload(fibitmap);
		return false;
	}

	LM_LOG_INFO("Successfully saved to " + imagePath);

	FreeImage_Unload(fibitmap);
	return true;
}

Film* HDRBitmapFilm::Clone() const
{
	auto* film = new HDRBitmapFilm;
	film->width = width;
	film->height = height;
	film->type = type;
	film->bitmap = bitmap;
	return film;
}

void HDRBitmapFilm::Allocate( int width, int height )
{
	this->width = width;
	this->height = height;
	bitmap.InternalData().assign(width * height * 3, Math::Float(0));
}

void HDRBitmapFilm::Rescale( const Math::Float& weight )
{
	auto& data = bitmap.InternalData();
	for (auto& v : data)
	{
		v *= weight;
	}
}

LM_COMPONENT_REGISTER_IMPL(HDRBitmapFilm, Film);

LM_NAMESPACE_END
