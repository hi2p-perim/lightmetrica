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