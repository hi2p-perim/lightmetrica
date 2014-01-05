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

#pragma once
#ifndef __LIB_NANON_CAMERA_H__
#define __LIB_NANON_CAMERA_H__

#include "asset.h"
#include "math.types.h"

NANON_NAMESPACE_BEGIN

class Film;
struct Ray;
struct Primitive;

/*!
	Camera.
	A base class of the cameras.
*/
class NANON_PUBLIC_API Camera : public Asset
{
public:

	Camera(const std::string& id);
	virtual ~Camera();

public:

	virtual std::string Name() const { return "camera"; }

public:

	/*!
		Convert the raster position to a ray.
		\param rasterPos Raster position in [0, 1]^2.
		\param ray Ray.
	*/
	virtual void RasterPosToRay(const Math::Vec2& rasterPos, Ray& ray) const = 0;

	/*!
		Get film.
		Returns the film referenced by the camera.
		\param Film.
	*/
	virtual Film* GetFilm() const = 0;
	
	/*!
		Register an reference to the primitive.
		Some implementation of camera needs transformed mesh information for sampling.
		The function registers the reference to the primitive.
		The function is internally called.
		\param primitives An instances of the primitive.
	*/
	virtual void RegisterPrimitive(const Primitive* primitive) = 0;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_CAMERA_H__