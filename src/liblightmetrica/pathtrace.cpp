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
#include <lightmetrica/pathtrace.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/random.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <pugixml.hpp>
#include <thread>
#include <omp.h>
#include <atomic>

LM_NAMESPACE_BEGIN

class PathtraceRenderer::Impl : public Object
{
public:

	Impl(PathtraceRenderer* self);

public:

	bool Configure( const pugi::xml_node& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	PathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	int numSamples;		// Number of samples
	int rrDepth;		// Depth of beginning RR
	int numThreads;		// Number of threads

};

PathtraceRenderer::Impl::Impl( PathtraceRenderer* self )
	: self(self)
{

}

bool PathtraceRenderer::Impl::Configure( const pugi::xml_node& node, const Assets& assets )
{
	// Check type
	if (node.attribute("type").as_string() != self->Type())
	{
		LM_LOG_ERROR(boost::str(boost::format("Invalid renderer type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// 'num_samples'
	auto numSamplesNode = node.child("num_samples");
	if (!numSamples)
	{
		numSamples = 1;
		LM_LOG_WARN(boost::str(boost::format("Using default value 'num_samples' = %d") % numSamples));
	}
	else
	{
		numSamples = std::stoi(numSamplesNode.child_value());
	}
	
	// 'rr_depth'
	auto rrDepthNode = node.child("rr_depth");
	if (!rrDepthNode)
	{
		rrDepth = 1;
		LM_LOG_WARN(boost::str(boost::format("Using default value 'rr_depth' = %d") % rrDepth));
	}
	else
	{
		rrDepth = std::stoi(rrDepthNode.child_value());
	}

	// 'num_threads'
	auto numThreadsNode = node.child("num_threads");
	if (!numThreadsNode)
	{
		numThreads = std::thread::hardware_concurrency();
		LM_LOG_WARN(boost::str(boost::format("Using default value 'num_threads' = %d") % numThreads));
	}
	else
	{
		numThreads = std::stoi(numThreadsNode.child_value());
		if (numThreads <= 0)
		{
			numThreads = std::thread::hardware_concurrency();
		}
	}

	return true;
}

bool PathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* film = scene.MainCamera()->GetFilm();
	std::atomic<int> processedLines;

	signal_ReportProgress(0, false);

	// Set number of threads
	omp_set_num_threads(numThreads);

	#pragma omp parallel for
	for (int y = 0; y < film->Height(); y++)
	{
		Ray ray;
		Intersection isect;
		Random rng(static_cast<int>(std::time(nullptr)) + omp_get_thread_num());

		for (int x = 0; x < film->Width(); x++)
		{
			for (int s = 0; s < numSamples; s++)
			{
				// Raster position
				Math::Vec2 rasterPos(
					(Math::Float(x) + rng.Next()) / Math::Float(film->Width()),
					(Math::Float(y) + rng.Next()) / Math::Float(film->Height()));

				// Generate camera ray
				scene.MainCamera()->RasterPosToRay(rasterPos, ray);

				Math::Vec3 L;
				Math::Vec3 throughput(Math::Float(1));
				int depth = 0;
				
				while (true)
				{
					// Check intersection
					if (!scene.Intersect(ray, isect))
					{
						//// There is no intersection, evaluate environment light if exists
						//const auto* envLight = scene.GetEnvironmentLight();
						//if (!envLight)
						//{
						//	L += throughput * envLight->Evaluate(-ray.d);
						//}
						break;
					}
					
					const auto* light = isect.primitive->light;
					if (light)
					{
						// Emission
						L += throughput * light->EvaluateLe(-ray.d, isect);
					}

					// --------------------------------------------------------------------------------

					// Sample BSDF
					const auto* bsdf = isect.primitive->bsdf;

					BSDFSampleQuery bsdfQuery;
					bsdfQuery.u = Math::Vec2(rng.Next(), rng.Next());
					bsdfQuery.type = BSDFType::All;
					bsdfQuery.transportDir = TransportDirection::CameraToLight;
					bsdfQuery.wi = isect.worldToShading * -ray.d;
					
					BSDFSampledData bsdfSampledData;
					if (!bsdf->SampleWo(bsdfQuery, bsdfSampledData) || bsdfSampledData.pdf.measure != ProbabilityMeasure::SolidAngle)
					{
						break;
					}

					auto fs = bsdf->Evaluate(BSDFEvaluateQuery(bsdfQuery, bsdfSampledData), isect);
					if (fs == Math::Vec3())
					{
						break;
					}
					
					// Update throughput
					// weight = f_s(w_i, w_o) * cos(theta_o) / p_\sigma(w_o), where theta_o is the angle between N_s and w_o.
					throughput *= fs * Math::CosThetaZUp(bsdfSampledData.wo) / bsdfSampledData.pdf.v;

					// Setup next ray
					ray.d = isect.shadingToWorld * bsdfSampledData.wo;
					ray.o = isect.p;
					ray.minT = Math::Constants::Eps();
					ray.maxT = Math::Constants::Inf();

					// --------------------------------------------------------------------------------

					if (++depth >= rrDepth)
					{
						// Russian roulette for path termination
						Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
						if (rng.Next() > p)
						{
							break;
						}

						throughput /= p;
					}
				}

				film->AccumulateContribution(rasterPos, L / Math::Float(numSamples));
			}
		}

		processedLines++;
		signal_ReportProgress(static_cast<double>(processedLines) / film->Height(), processedLines == film->Height());
	}

	return true;
}

// --------------------------------------------------------------------------------

PathtraceRenderer::PathtraceRenderer()
	: p(new Impl(this))
{

}

PathtraceRenderer::~PathtraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool PathtraceRenderer::Configure( const pugi::xml_node& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool PathtraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection PathtraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END