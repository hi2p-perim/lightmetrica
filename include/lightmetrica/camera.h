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
class LM_PUBLIC_API Camera : public Emitter
{
public:

	Camera(const std::string& id);
	virtual ~Camera();

public:

	virtual std::string Name() const { return "camera"; }

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