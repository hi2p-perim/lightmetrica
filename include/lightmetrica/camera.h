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
#ifndef LIB_LIGHTMETRICA_CAMERA_H
#define LIB_LIGHTMETRICA_CAMERA_H

#include "emitter.h"

LM_NAMESPACE_BEGIN

class Film;
struct Ray;

/*!
	Camera.
	A base class of the cameras.
*/
class Camera : public Emitter
{
public:

	LM_ASSET_INTERFACE_DEF("camera", "cameras");
	LM_ASSET_DEPENDENCIES("film");

public:

	Camera() {}
	virtual ~Camera() {}

public:

	/*!
		Convert a ray to a raster position.
		The function calculates the raster position from the outgoing ray.
		Returns false if calculated raster position is the outside of [0, 1]^2.
		\param p Position on the camera.
		\param d Outgoing direction from #p.
		\param rasterPos Raster position.
		\return true Succeeded to convert.
		\return false Failed to convert.
	*/
	virtual bool RayToRasterPosition(const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos) const = 0;

	/*!
		Get film.
		Returns the film referenced by the camera.
		\param Film.
	*/
	virtual Film* GetFilm() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_CAMERA_H