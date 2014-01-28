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
#include <lightmetrica/lighttrace.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/random.h>
#include <lightmetrica/light.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/bsdf.h>
#include <pugixml.hpp>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

class LighttraceRenderer::Impl : public Object
{
public:

	Impl(LighttraceRenderer* self);

public:

	bool Configure( const pugi::xml_node& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	LighttraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	int numSamples;		// Number of samples
	int rrDepth;		// Depth of beginning RR
	int numThreads;		// Number of threads

};

LighttraceRenderer::Impl::Impl( LighttraceRenderer* self )
	: self(self)
{

}

bool LighttraceRenderer::Impl::Configure( const pugi::xml_node& node, const Assets& assets )
{
	// Check type
	if (node.attribute("type").as_string() != self->Type())
	{
		LM_LOG_ERROR(boost::str(boost::format("Invalid renderer type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// 'num_samples'
	auto numSamplesNode = node.child("num_samples");
	if (!numSamplesNode)
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

bool LighttraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<int> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Random>> rngs;
	std::vector<std::unique_ptr<Film>> films;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		rngs.push_back(std::unique_ptr<Random>(new Random(seed + i)));
		films.push_back(std::unique_ptr<Film>(masterFilm->Clone()));
	}

	// Number of blocks to be separated
	const int samplesPerBlock = 100;
	int blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (int block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = rngs[threadId];
		auto& film = films[threadId];

		// Sample range
		int sampleBegin = samplesPerBlock * block;
		int sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		for (int sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Sample a light

			// Random number for light sampling
			auto ls = rng->NextVec2();

			// Choose a light
			int nl = scene.NumLights();
			int li = Math::Min(static_cast<int>(ls.x * nl), nl - 1);
			ls.x = ls.x * nl - Math::Float(li);
			const Light* light = scene.LightByIndex(li);
			Math::PDFEval lightSelectionPdf(Math::Float(1.0 / li), Math::ProbabilityMeasure::Discrete);

			// Sample a position and a direction on the light
			LightSampleQuery lightSQ;
			lightSQ.sampleP = ls;
			lightSQ.sampleD = rng->NextVec2();

			LightSampleResult lightSR;
			light->Sample(lightSQ, lightSR);

			// Evaluate Le
			auto Le = light->EvaluateLe(lightSR.d, lightSR.gn);

			// --------------------------------------------------------------------------------

			// Handle LE path
			// TODO

			// --------------------------------------------------------------------------------

			// Construct a ray
			Ray ray;
			ray.d = lightSR.d;
			ray.o = lightSR.p;
			ray.minT = Math::Constants::Eps();
			ray.maxT = Math::Constants::Inf();

			// --------------------------------------------------------------------------------

			// Trace light particle and evaluate importance
			Math::Vec3 throughput(Math::Float(1));
			Intersection isect;
			int depth = 0;

			while (true)
			{
				// Query intersection
				if (!scene.Intersect(ray, isect))
				{
					break;
				}

				// ----------------------------------------------------------------------

				// Sample a position
				Math::Vec3 ep;
				Math::PDFEval pdfEp;
				scene.MainCamera()->SamplePosition(rng->NextVec2(), ep, pdfEp);
				if (!Math::IsZero(pdfEp.v))
				{
					// Calculate raster position
					Math::Vec2 rasterPos;
					if (scene.MainCamera()->RayToRasterPosition(ep, isect.p - ep, rasterPos))
					{
						// Evaluate importance
						auto We = scene.MainCamera()->EvaluateWe(ep, isect.p - ep);
						if (!Math::IsZero(We))
						{
							// Check visibility between camera position and sampled position in the light
							Ray shadowRay;
							auto d = ep - isect.p;
							Math::Float dl = Math::Length(d);
							shadowRay.d = d / dl;
							shadowRay.o = isect.p;
							shadowRay.minT = Math::Constants::Eps();
							shadowRay.maxT = dl * (Math::Float(1) - Math::Constants::Eps());

							Intersection shadowIsect;
							if (!scene.Intersect(shadowRay, shadowIsect))
							{
								// Evaluate BSDF
								BSDFEvaluateQuery bsdfEQ;
								bsdfEQ.transportDir = TransportDirection::LightToCamera;
								bsdfEQ.type = BSDFType::All;
								bsdfEQ.wi = isect.worldToShading * -ray.d;
								bsdfEQ.wo = isect.worldToShading * shadowRay.d;

								auto bsdf = isect.primitive->bsdf->Evaluate(bsdfEQ, isect);
								if (!Math::IsZero(bsdf))
								{
									// Accumulate color
									film->AccumulateContribution(rasterPos, Le * throughput * bsdf * We / pdfEp.v);
								}
							}
						}
					}
				}

				// ----------------------------------------------------------------------

				// Sample BSDF
				BSDFSampleQuery bsdfSQ;
				bsdfSQ.sample = rng->NextVec2();
				bsdfSQ.type = BSDFType::All;
				bsdfSQ.wi = isect.worldToShading * -ray.d;
				bsdfSQ.transportDir = TransportDirection::LightToCamera;

				BSDFSampleResult bsdfSR;
				if (!isect.primitive->bsdf->Sample(bsdfSQ, bsdfSR) || bsdfSR.pdf.measure != Math::ProbabilityMeasure::SolidAngle)
				{
					break;
				}

				// Evaluate BSDF
				auto bsdf = isect.primitive->bsdf->Evaluate(BSDFEvaluateQuery(bsdfSQ, bsdfSR), isect);
				if (Math::IsZero(bsdf))
				{
					break;
				}

				// Update throughput
				throughput *= bsdf;

				// Setup next ray
				ray.d = isect.shadingToWorld * bsdfSR.wo;
				ray.o = isect.p;
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// ----------------------------------------------------------------------

				if (++depth >= rrDepth)
				{
					// Russian roulette for path termination
					Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
					if (rng->Next() > p)
					{
						break;
					}

					throughput /= p;
				}
			}
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(f.get());
	}

	return true;
}

// --------------------------------------------------------------------------------

LighttraceRenderer::LighttraceRenderer()
	: p(new Impl(this))
{

}

LighttraceRenderer::~LighttraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool LighttraceRenderer::Configure( const pugi::xml_node& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool LighttraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection LighttraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END