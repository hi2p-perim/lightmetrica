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
#ifndef __LIB_LIGHTMETRICA_HDR_FILM_H__
#define __LIB_LIGHTMETRICA_HDR_FILM_H__

#include "film.h"

LM_NAMESPACE_BEGIN

/*!
	HDR image types.
*/
enum class HDRImageType
{
	RadianceHDR,	// Radiance HDR
	OpenEXR			// OpenEXR
};

/*!
	High dynamic range bitmap film.
	Implements HDR version of bitmap image recording.
*/
class LM_PUBLIC_API HDRBitmapFilm : public Film
{
public:

	HDRBitmapFilm(const std::string& id);
	virtual ~HDRBitmapFilm();

public:

	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets );
	virtual std::string Type() const { return "hdr"; }
	
public:

	virtual int Width() const;
	virtual int Height() const;
	virtual bool Save(const std::string& path) const;
	virtual void RecordContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb);
	virtual void AccumulateContribution( const Math::Vec2& rasterPos, const Math::Vec3& contrb );
	virtual void AccumulateContribution( const Film* film );

public:

	virtual Film* Clone() const;

public:

	/*!
		Allocate the film with given width and height.
		\param width Width of the film.
		\param height Height of the film.
	*/
	void Allocate(int width, int height);

	/*!
		Set image type.
		Set HDR image type.
		\param type Image type.
	*/
	void SetImageType(HDRImageType type);

	/*!
		Get the internal data.
		Copy the internal data to the given array #dest.
		This function is used internally for testing.
		\param dest An array to store internal data.
	*/
	void InternalData(std::vector<Math::Float>& dest);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_HDR_FILM_H__
