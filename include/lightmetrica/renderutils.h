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
#ifndef LIB_LIGHTMETRICA_RENDER_UTILS_H
#define LIB_LIGHTMETRICA_RENDER_UTILS_H

#include "common.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

struct SurfaceGeometry;
class Scene;

/*!
	Render utilities.
	Utilities for renderer implementations.
*/
class LM_PUBLIC_API RenderUtils
{
private:

	RenderUtils();
	~RenderUtils();

	LM_DISABLE_COPY_AND_MOVE(RenderUtils);

public:

	/*!
		Compute generalized geometry term.
		Ordinary geometry term + degeneration support.
		Note that the geometry term is commutable.
		\param geom1 Surface geometry for eye sub-path.
		\param geom2 Surface geometry for light sub-path.
		\return Generalized geometry term.
	*/
	static Math::Float GeneralizedGeometryTerm(const SurfaceGeometry& geom1, const SurfaceGeometry& geom2);

	/*!
		Compute generalized geometry term including visibility.
		Use this function if explicit computation of visibility term is needed.
		\param scene Scene.
		\param geom1 Surface geometry for eye sub-path.
		\param geom2 Surface geometry for light sub-path.
		\return Generalized geometry term.
	*/
	static Math::Float GeneralizedGeometryTermWithVisibility(const Scene& scene, const SurfaceGeometry& geom1, const SurfaceGeometry& geom2);

	/*!
		Check visibility between two points.
		\param scene Scene.
		\param p1 First point.
		\param p2 Second point.
		\retval true Two points are mutually visible.
		\retval false Two points are not mutually visible.
	*/
	static bool CheckVisibility(const Scene& scene, const Math::Vec3& p1, const Math::Vec3& p2);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RENDER_UTILS_H