/*
	nanon : A research-oriented renderer

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
#include <nanon/hdrfilm.h>
#include <nanon/logger.h>
#include <pugixml.hpp>
#include <FreeImage.h>

NANON_NAMESPACE_BEGIN

enum class ImageType
{
	RadianceHDR,	// Radiance HDR
	OpenEXR			// OpenEXR
};

class HDRBitmapFilm::Impl : public Object
{
public:

	Impl(HDRBitmapFilm* self);
	bool LoadAsset(const pugi::xml_node& node, const Assets& assets);
	int Width() const { return width; }
	int Height() const { return height; }
	bool Save();
	void RecordContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb);
	void AccumulateContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb);
	void InternalData(std::vector<Math::Float>& dest);

public:

	static void FreeImageErrorCallback(FREE_IMAGE_FORMAT fif, const char* message);

private:

	HDRBitmapFilm* self;
	int width;
	int height;
	std::string path;				// Path to the image to be saved
	ImageType type;					// Type of the image to be saved
	std::vector<Math::Float> data;	// Image data

};

HDRBitmapFilm::Impl::Impl( HDRBitmapFilm* self )
	: self(self)
{
	
}

bool HDRBitmapFilm::Impl::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	// Check name and type
	if (node.name() != self->Name())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid node name '%s'") % node.name()));
		return false;
	}

	if (node.attribute("type").as_string() != self->Type())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid film type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// Find 'width' element
	auto widthNode = node.child("width");
	if (!widthNode)
	{
		NANON_LOG_ERROR("Missing 'width' element");
		return false;
	}

	// Find 'height' element
	auto heightNode = node.child("height");
	if (!heightNode)
	{
		NANON_LOG_ERROR("Missing 'height' element");
		return false;
	}

	// Find 'path' element
	auto pathNode = node.child("path");
	if (!pathNode)
	{
		NANON_LOG_ERROR("Missing 'path' element");
		return false;
	}

	// Store values
	width = std::stoi(widthNode.child_value());
	height = std::stoi(heightNode.child_value());
	path = pathNode.child_value();

	// Find 'image_type' element (optional)
	auto imageTypeNode = node.child("imagetype");
	if (!imageTypeNode)
	{
		// Use .hdr as default type
		type = ImageType::RadianceHDR;
	}
	else
	{
		if (std::strcmp("radiancehdr", imageTypeNode.child_value()) == 0)
		{
			// Image type is .hdr (Radiance HDR)
			type = ImageType::RadianceHDR;
		}
		else if (std::strcmp("openexr", imageTypeNode.child_value()) == 0)
		{
			// Image type is .exr (OpenEXR)
			type = ImageType::OpenEXR;
		}
		else
		{
			NANON_LOG_ERROR(boost::str(boost::format("Invalid image type '%s'") % imageTypeNode.child_value()));
			return false;
		}
	}

	// Initialize image data
	data.assign(width * height * 3, Math::Float(0));

	// Error handing of FreeImage
	FreeImage_SetOutputMessage(FreeImageErrorCallback);

	return true;
}

void HDRBitmapFilm::Impl::FreeImageErrorCallback( FREE_IMAGE_FORMAT fif, const char* message )
{
	std::string format = fif != FIF_UNKNOWN
		? FreeImage_GetFormatFromFIF(fif)
		: "unknown";

	NANON_LOG_ERROR(format);
	NANON_LOG_ERROR(message);
}

void HDRBitmapFilm::Impl::RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	// Convert raster position to pixel position
	Math::Vec2i pixelPos(
		Math::Cast<int>(Math::Float(rasterPos.x * Math::Float(width))),
		Math::Cast<int>(Math::Float(rasterPos.y * Math::Float(height))));

	if (pixelPos.x < 0 || width <= pixelPos.x || pixelPos.y < 0 || height <= pixelPos.y)
	{
		NANON_LOG_WARN(boost::str(boost::format("Invalid pixel position (%d, %d)") % pixelPos.x % pixelPos.y));
		return;
	}

	// Record contribution
	size_t idx = pixelPos.y * width + pixelPos.x;
	data[3 * idx    ] = contrb[0];
	data[3 * idx + 1] = contrb[1];
	data[3 * idx + 2] = contrb[2];
}

void HDRBitmapFilm::Impl::AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	// Convert raster position to pixel position
	Math::Vec2i pixelPos(
		Math::Cast<int>(Math::Float(rasterPos.x * Math::Float(width))),
		Math::Cast<int>(Math::Float(rasterPos.y * Math::Float(height))));

	if (pixelPos.x < 0 || width <= pixelPos.x || pixelPos.y < 0 || height <= pixelPos.y)
	{
		NANON_LOG_WARN(boost::str(boost::format("Invalid pixel position (%d, %d)") % pixelPos.x % pixelPos.y));
		return;
	}

	// Accumulate contribution
	size_t idx = pixelPos.y * width + pixelPos.x;
	data[3 * idx    ] += contrb[0];
	data[3 * idx + 1] += contrb[1];
	data[3 * idx + 2] += contrb[2];
}

bool HDRBitmapFilm::Impl::Save()
{
	// Create bitmap
	// 128 bit RGBA float image
	// Note: EXR - FIT_RGBAF, HDR - FIT_RGBF
	FIBITMAP* bitmap = FreeImage_AllocateT(FIT_RGBF, width, height);
	if (!bitmap)
	{
		NANON_LOG_ERROR("Failed to allocate bitmap");
		return false;
	}

	// Copy data
	for (int y = 0; y < height; y++)
	{
		FIRGBF* bits = (FIRGBF*)FreeImage_GetScanLine(bitmap, y);
		for (int x = 0; x < width; x++)
		{
			int idx = y * width + x;
			bits[x].red		= Math::Cast<float>(data[3 * idx    ]);
			bits[x].green	= Math::Cast<float>(data[3 * idx + 1]);
			bits[x].blue	= Math::Cast<float>(data[3 * idx + 2]);
		}
	}

	BOOL result;
	if (type == ImageType::RadianceHDR)
	{
		// Save image as Radiance HDR format
		result = FreeImage_Save(FIF_HDR, bitmap, path.c_str(), HDR_DEFAULT);
	}
	else if (type == ImageType::OpenEXR)
	{
		// Save image as OpenEXR format
		result = FreeImage_Save(FIF_EXR, bitmap, path.c_str(), EXR_DEFAULT);
	}

	if (!result)
	{
		NANON_LOG_DEBUG("Failed to save image : " + path);
		FreeImage_Unload(bitmap);
		return false;
	}

	FreeImage_Unload(bitmap);
	return true;
}

void HDRBitmapFilm::Impl::InternalData( std::vector<Math::Float>& dest )
{
	dest.clear();
	dest.resize(data.size());
	std::copy(data.begin(), data.end(), dest.begin());
}

// --------------------------------------------------------------------------------

HDRBitmapFilm::HDRBitmapFilm(const std::string& id)
	: Film(id)
	, p(new Impl(this))
{

}

HDRBitmapFilm::~HDRBitmapFilm()
{
	NANON_SAFE_DELETE(p);
}

bool HDRBitmapFilm::Save() const
{
	return p->Save();
}

void HDRBitmapFilm::InternalData( std::vector<Math::Float>& dest )
{
	p->InternalData(dest);
}

int HDRBitmapFilm::Width() const
{
	return p->Width();
}

int HDRBitmapFilm::Height() const
{
	return p->Height();
}

void HDRBitmapFilm::RecordContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	p->RecordContribution(rasterPos, contrb);
}

void HDRBitmapFilm::AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb )
{
	p->AccumulateContribution(rasterPos, contrb);
}

bool HDRBitmapFilm::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

NANON_NAMESPACE_END