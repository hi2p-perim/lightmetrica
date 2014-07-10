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

#include "pch.h"
#include <lightmetrica/renderutils.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/scene.h>

LM_NAMESPACE_BEGIN

Math::Float RenderUtils::GeneralizedGeometryTerm( const SurfaceGeometry& geom1, const SurfaceGeometry& geom2 )
{
	auto p1p2 = geom2.p - geom1.p;
	auto p1p2_Length2 = Math::Length2(p1p2);
	auto p1p2_Length = Math::Sqrt(p1p2_Length2);
	p1p2 /= p1p2_Length;

	// Be careful to use shading normals instead of geometry normals
	Math::Float numerator(1);
	if (!geom1.degenerated)
	{
		numerator *= Math::Abs(Math::Dot(geom1.sn, p1p2));
	}
	if (!geom2.degenerated)
	{
		numerator *= Math::Abs(Math::Dot(geom2.sn, -p1p2));
	}

	// Cope with a singularity in the geometry term evaluation
	// some numerical issues are introduce due to this singularity.
	// TODO : Automatically select appropriate tolerance
	if (Math::Abs(p1p2_Length) < Math::Constants::Eps())
	{
		return Math::Float(0);
	}

	return numerator / p1p2_Length2;
}

bool RenderUtils::Visible( const Scene& scene, const Math::Vec3& p1, const Math::Vec3& p2 )
{
	Ray shadowRay;
	auto p1p2 = p2 - p1;
	auto p1p2_Length = Math::Length(p1p2);
	shadowRay.d = p1p2 / p1p2_Length;
	shadowRay.o = p1;
	shadowRay.minT = Math::Constants::Eps();
	shadowRay.maxT = p1p2_Length * (Math::Float(1) - Math::Constants::Eps());

	Intersection _;
	return !scene.Intersect(shadowRay, _);
}

Math::Float RenderUtils::GeneralizedGeometryTermWithVisibility( const Scene& scene, const SurfaceGeometry& geom1, const SurfaceGeometry& geom2 )
{
	if (!Visible(scene, geom1.p, geom2.p))
	{
		return Math::Float(0);
	}

	return GeneralizedGeometryTerm(geom1, geom2);
}

LM_NAMESPACE_END