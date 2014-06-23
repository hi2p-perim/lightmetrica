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
#include <lightmetrica/renderer.h>
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

class RaycastRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("raycast");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Render(const Scene& scene);
	virtual bool Preprocess( const Scene& scene ) { signal_ReportProgress(0, true); return true; }
	virtual bool Configure(const ConfigNode& node, const Assets& assets);
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;
	int numThreads;

};

bool RaycastRenderer::Render(const Scene& scene)
{
	auto* film = scene.MainCamera()->GetFilm();
	std::atomic<int> processedLines(0);

	signal_ReportProgress(0, false);

	// Set number of threads
	omp_set_num_threads(numThreads);

	#pragma omp parallel for
	for (int y = 0; y < film->Height(); y++)
	{
		for (int x = 0; x < film->Width(); x++)
		{
			// Raster position
			Math::Vec2 rasterPos(
				(Math::Float(0.5) + Math::Float(x)) / Math::Float(film->Width()),
				(Math::Float(0.5) + Math::Float(y)) / Math::Float(film->Height()));

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

		processedLines++;
		signal_ReportProgress(static_cast<double>(processedLines) / film->Height(), processedLines == film->Height());
	}

	return true;
}

bool RaycastRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	// Load parameters
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}

	return true;
}

LM_COMPONENT_REGISTER_IMPL(RaycastRenderer, Renderer);

LM_NAMESPACE_END
