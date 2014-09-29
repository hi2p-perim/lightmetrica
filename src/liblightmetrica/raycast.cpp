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
#include <lightmetrica/renderer.h>
#include <lightmetrica/renderproc.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/math.functions.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <atomic>
#include <thread>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Raycast renderer.
	Implements raycasting.
	This renderer is utilized for testing.
*/
class RaycastRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("raycast");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene) { return true; }
	virtual bool Preprocess(const Scene& scene) { signal_ReportProgress(1, true); return true; }
	virtual bool Postprocess(const Scene& scene) const { return true; }
	virtual RenderProcess* CreateRenderProcess(const Scene& scene, int threadID, int numThreads);
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;
	int numThreads;

};

// --------------------------------------------------------------------------------

/*!
	Render process for RaycastRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class RaycastRenderer_RenderProcess : public DeterministicPixelBasedRenderProcess
{
public:

	RaycastRenderer_RenderProcess() {}

private:

	LM_DISABLE_COPY_AND_MOVE(RaycastRenderer_RenderProcess);

public:

	virtual void ProcessSinglePixel(const Scene& scene, const Math::Vec2i& pixel);

};

// --------------------------------------------------------------------------------

RenderProcess* RaycastRenderer::CreateRenderProcess(const Scene& scene, int threadID, int numThreads)
{
	return new RaycastRenderer_RenderProcess();
}

// --------------------------------------------------------------------------------

void RaycastRenderer_RenderProcess::ProcessSinglePixel(const Scene& scene, const Math::Vec2i& pixel)
{
	auto* film = scene.MainCamera()->GetFilm();

	// Raster position
	Math::Vec2 rasterPos(
		(Math::Float(0.5) + Math::Float(pixel.x)) / Math::Float(film->Width()),
		(Math::Float(0.5) + Math::Float(pixel.y)) / Math::Float(film->Height()));

	// Generate ray
	// Note : position sampling is not used here (thus DoF is disabled)
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE, pdfDE;
	scene.MainCamera()->SamplePosition(Math::Vec2(), geomE, pdfPE);

	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;
	bsdfSQ.sample = rasterPos;
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	scene.MainCamera()->SampleDirection(bsdfSQ, geomE, bsdfSR);

	Ray ray;
	ray.d = bsdfSR.wo;
	ray.o = geomE.p;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	// Check intersection
	Intersection isect;
	if (scene.Intersect(ray, isect))
	{
		// Intersected : while color
		Math::Float c = Math::Abs(Math::Dot(isect.geom.sn, -ray.d));
		film->RecordContribution(rasterPos, Math::Vec3(c));
	}
	else
	{
		// Not intersected : black color
		film->RecordContribution(rasterPos, Math::Colors::Black());
	}
}

LM_COMPONENT_REGISTER_IMPL(RaycastRenderer, Renderer);

LM_NAMESPACE_END
