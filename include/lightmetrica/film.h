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
#ifndef __LIB_LIGHTMETRICA_FILM_H__
#define __LIB_LIGHTMETRICA_FILM_H__

#include "asset.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

/*!
	Film.
	A base class of the films.
	The class is used to rendered images equipped with cameras.
*/
class LM_PUBLIC_API Film : public Asset
{
public:

	Film(const std::string& id);
	virtual ~Film();

public:

	std::string Name() const { return "film"; }

public:

	/*!
		Get the width of the film.
		\retval Width of the film.
	*/
	virtual int Width() const = 0;

	/*!
		Get the height of the film.
		\retval Height of the film.
	*/
	virtual int Height() const = 0;

	/*!
		Save as image.
		Saves the film as image.
		If #path is empty, the default path is used.
		\param path Path to the output image.
	*/
	virtual bool Save(const std::string& path) const = 0;

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
		\param film A film.
	*/
	virtual void AccumulateContribution(const Film* film) = 0;

public:

	/*!
		Clone the film.
		The function is used internally.
		\return Duplicated film.
	*/
	virtual Film* Clone() const = 0;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_FILM_H__