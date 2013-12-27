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

#ifndef __LIB_NANON_PERSPECTIVE_CAMERA_H__
#define __LIB_NANON_PERSPECTIVE_CAMERA_H__

#include "camera.h"

NANON_NAMESPACE_BEGIN

/*!
	Perspective camera.
	A camera with perspective projection. a.k.a. pinhole camera.
*/
class NANON_PUBLIC_API PerspectiveCamera : public Camera
{
public:

	PerspectiveCamera(const std::string& id);
	~PerspectiveCamera();

public:

	virtual bool Load( const pugi::xml_node& node, const Assets& assets );
	virtual std::string Type() const { return "perspective"; }

public:

	virtual void RasterPosToRay( const Math::Vec2& rasterPos, Ray& ray ) const;
	virtual const Film* GetFilm() const;
	virtual void RegisterPrimitive( Primitive* primitive );

private:

	class Impl;
	Impl* p;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_PERSPECTIVE_CAMERA_H__