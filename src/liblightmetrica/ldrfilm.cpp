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

#include "pch.h"
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/bitmap.h>
#include <FreeImage.h>

LM_NAMESPACE_BEGIN

class LDRBitmapFilm : public BitmapFilm
{
public:

	LM_COMPONENT_IMPL_DEF("ldr");

public:

	LDRBitmapFilm() {}
	virtual ~LDRBitmapFilm() {}

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
	virtual void Clear();

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

bool LDRBitmapFilm::Load( const ConfigNode& node, const Assets& assets )
{
	if (!node.ChildValue("width",	width))		return false;
	if (!node.ChildValue("height",	height))	return false;

	// Find 'image_type' element (optional)
	auto imageTypeNode = node.Child("imagetype");
	if (imageTypeNode.Empty())
	{
		// Use .png as default type
		SetImageType(BitmapImageType::PNG);
	}
	else if (imageTypeNode.Value() == "png")
	{
		SetImageType(BitmapImageType::PNG);
	}
	else
	{
		LM_LOG_ERROR("Invalid image type '" + imageTypeNode.Value() + "'");
		return false;
	}

	// Allocate image data
	Allocate(width, height);

	return true;
}

void LDRBitmapFilm::FreeImageErrorCallback( FREE_IMAGE_FORMAT fif, const char* message )
{
	std::string format = fif != FIF_UNKNOWN
		? FreeImage_GetFormatFromFIF(fif)
		: "unknown";

	LM_LOG_ERROR(format);
	LM_LOG_ERROR(message);
}

// TODO : Same function as HDRFilm
void LDRBitmapFilm::RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
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

void LDRBitmapFilm::AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
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

void LDRBitmapFilm::AccumulateContribution( const Film& film )
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
	const auto& otherData = dynamic_cast<const LDRBitmapFilm&>(film).bitmap.InternalData();
	auto& data = bitmap.InternalData();
	LM_ASSERT(data.size() == otherData.size());
	for (size_t i = 0; i < data.size(); i++)
	{
		data[i] += otherData[i];
	}
}

bool LDRBitmapFilm::RescaleAndSave( const std::string& path, const Math::Float& weight ) const
{
	// Error handing of FreeImage
	FreeImage_SetOutputMessage(FreeImageErrorCallback);

	// If #path is empty, the default path is used
	std::string imagePath = path;
	if (imagePath.empty())
	{
		imagePath = "result.png";
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

	// ----------------------------------------------------------------------

	// Tone mapping
	//auto* tonemappedBitmap = FreeImage_ToneMapping(fibitmap, FITMO_FATTAL02);
	//auto* tonemappedBitmap = FreeImage_TmoDrago03(fibitmap);

	FIBITMAP* tonemappedBitmap = FreeImage_Allocate(width, height, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
	if (!tonemappedBitmap)
	{
		LM_LOG_ERROR("Failed to allocate bitmap");
		return false;
	}

	// Multiplier for gamma correction
	const double Exp = 1.0 / 2.2;

	// Number of bytes per pixel
	const int Bytespp = 3;
	for (int y = 0; y < height; y++)
	{
		BYTE* bits = FreeImage_GetScanLine(tonemappedBitmap, y);
		for (int x = 0; x < width; x++)
		{
			int idx = y * width + x;
			bits[FI_RGBA_RED]   = static_cast<BYTE>(Math::Clamp(Math::Pow(Math::Cast<double>(data[3 * idx    ] * weight), Exp), 0.0, 1.0) * 255.0);
			bits[FI_RGBA_GREEN] = static_cast<BYTE>(Math::Clamp(Math::Pow(Math::Cast<double>(data[3 * idx + 1] * weight), Exp), 0.0, 1.0) * 255.0);
			bits[FI_RGBA_BLUE]  = static_cast<BYTE>(Math::Clamp(Math::Pow(Math::Cast<double>(data[3 * idx + 2] * weight), Exp), 0.0, 1.0) * 255.0);
			bits += Bytespp;
		}
	}

	// ----------------------------------------------------------------------

	// Save image
	BOOL result = false;
	if (type == BitmapImageType::PNG)
	{
		result = FreeImage_Save(FIF_PNG, tonemappedBitmap, imagePath.c_str(), PNG_DEFAULT);;
	}

	if (!result)
	{
		LM_LOG_DEBUG("Failed to save image : " + imagePath);
		FreeImage_Unload(fibitmap);
		FreeImage_Unload(tonemappedBitmap);
		return false;
	}

	LM_LOG_INFO("Successfully saved to " + imagePath);

	FreeImage_Unload(fibitmap);
	FreeImage_Unload(tonemappedBitmap);

	return true;
}

Film* LDRBitmapFilm::Clone() const
{
	auto* film = new LDRBitmapFilm;
	film->width = width;
	film->height = height;
	film->type = type;
	film->bitmap = bitmap;
	return film;
}

void LDRBitmapFilm::Clear()
{
	auto& data = bitmap.InternalData();
	for (auto& v : data)
	{
		v = Math::Float(0);
	}
}

void LDRBitmapFilm::Allocate( int width, int height )
{
	this->width = width;
	this->height = height;
	bitmap.InternalData().assign(width * height * 3, Math::Float(0));
}

void LDRBitmapFilm::Rescale( const Math::Float& weight )
{
	auto& data = bitmap.InternalData();
	for (auto& v : data)
	{
		v *= weight;
	}
}

LM_COMPONENT_REGISTER_IMPL(LDRBitmapFilm, Film);

LM_NAMESPACE_END