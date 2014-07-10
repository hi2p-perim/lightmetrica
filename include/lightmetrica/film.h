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