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

#include "pch.h"
#include <nanon/raycast.h>
#include <nanon/scene.h>
#include <nanon/camera.h>
#include <nanon/film.h>
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <nanon/math.functions.h>
#include <nanon/logger.h>

NANON_NAMESPACE_BEGIN

class RaycastRenderer::Impl
{
public:

	bool Render(const Scene& scene);
	bool Configure(const pugi::xml_node& node, const Assets& assets);

};

bool RaycastRenderer::Impl::Render(const Scene& scene)
{
	auto* film = scene.MainCamera()->GetFilm();

	#pragma omp parallel for
	for (int y = 0; y < film->Height(); y++)
	{
		Ray ray;
		Intersection isect;

		for (int x = 0; x < film->Width(); x++)
		{
			// Raster position
			Math::Vec2 rasterPos(
				(Math::Float(0.5) + Math::Float(x)) / Math::Float(film->Width()),
				(Math::Float(0.5) + Math::Float(y)) / Math::Float(film->Height()));

			// Generate ray
			scene.MainCamera()->RasterPosToRay(rasterPos, ray);

			// Check intersection
			if (scene.Intersect(ray, isect))
			{
				// Intersected : while color
				film->RecordContribution(rasterPos, Math::Vec3(Math::Abs(Math::Dot(isect.sn, -ray.d))));
			}
			else
			{
				// Not intersected : black color
				film->RecordContribution(rasterPos, Math::Colors::Black);
			}
		}
	}

	if (!film->Save())
	{
		NANON_LOG_DEBUG_EMPTY();
		return false;
	}

	return true;
}

bool RaycastRenderer::Impl::Configure( const pugi::xml_node& node, const Assets& assets )
{
	// Nothing!!
	return true;
}

// --------------------------------------------------------------------------------

RaycastRenderer::RaycastRenderer()
	: p(new Impl)
{

}

RaycastRenderer::~RaycastRenderer()
{
	NANON_SAFE_DELETE(p);
}

bool RaycastRenderer::Render(const Scene& scene)
{
	return p->Render(scene);
}

bool RaycastRenderer::Configure( const pugi::xml_node& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

NANON_NAMESPACE_END