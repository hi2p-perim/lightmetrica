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
	static bool Visible(const Scene& scene, const Math::Vec3& p1, const Math::Vec3& p2);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_RENDER_UTILS_H