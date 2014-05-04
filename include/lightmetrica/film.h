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
#ifndef LIB_LIGHTMETRICA_FILM_H
#define LIB_LIGHTMETRICA_FILM_H

#include "asset.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

/*!
	Film.
	A base class of the films.
	The class is used to rendered images equipped with cameras.
*/
class Film : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("film", "films");
	LM_ASSET_NO_DEPENDENCIES();

public:

	Film() {}
	virtual ~Film() {}

public:

	/*!
		Get the width of the film.
		\return Width of the film.
	*/
	virtual int Width() const = 0;

	/*!
		Get the height of the film.
		\return Height of the film.
	*/
	virtual int Height() const = 0;

	/*!
		Record the contribution to the raster position.
		This function records #contrb to the position specified by #rasterPos.
		\param rasterPos Raster position.
		\param contrb Contribution.
	*/
	virtual void RecordContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb) = 0;

	/*!
		Accumulate the contribution to the raster position.
		This function accumulates #contrb to the position specified by #rasterPos.
		\param rasterPos Raster position.
		\param contrb Contribution.
	*/
	virtual void AccumulateContribution(const Math::Vec2& rasterPos, const Math::Vec3& contrb) = 0;

	/*!
		Accumulate the contribution to the entire film.
		This function accumulates the contribution of the other film.
		The other film must be same size and type.
		\param film Other film.
	*/
	virtual void AccumulateContribution(const Film& film) = 0;

	/*!
		Rescale the pixel values by constant weight.
		\param weight Rescaling weight.
	*/
	virtual void Rescale(const Math::Float& weight) = 0;

	/*!
		Clone the film.
		\return Duplicated film.
	*/
	virtual Film* Clone() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_FILM_H