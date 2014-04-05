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
#include <lightmetrica/randomfactory.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

class PathtraceRenderer::Impl : public Object
{
public:

	Impl(PathtraceRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	PathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;		// Number of samples
	int rrDepth;				// Depth of beginning RR
	int numThreads;				// Number of threads
	long long samplesPerBlock;	// Samples to be processed per block
	std::string rngType;		// Type of random number generator

};

PathtraceRenderer::Impl::Impl( PathtraceRenderer* self )
	: self(self)
{

}

bool PathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	// Check type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid renderer type '" + node.AttributeValue("type") + "'");
		return false;
	}

	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}
	node.ChildValueOrDefault("samples_per_block", 100LL, samplesPerBlock);
	if (samplesPerBlock <= 0)
	{
		LM_LOG_ERROR("Invalid value for 'samples_per_block'");
		return false;
	}
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!RandomFactory::CheckSupport(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

	return true;
}

bool PathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

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
		rngs.emplace_back(RandomFactory::Create(rngType));
		rngs.back()->SetSeed(seed + i);
		films.emplace_back(masterFilm->Clone());
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = rngs[threadId];
		auto& film = films[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Raster position
			auto rasterPos = rng->NextVec2();

			// Sample position on camera
			SurfaceGeometry geomE;
			Math::PDFEval pdfP;
			scene.MainCamera()->SamplePosition(rng->NextVec2(), geomE, pdfP);

			// Sample ray direction
			GeneralizedBSDFSampleQuery bsdfSQ;
			GeneralizedBSDFSampleResult bsdfSR;
			bsdfSQ.sample = rasterPos;
			bsdfSQ.transportDir = TransportDirection::EL;
			bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
			scene.MainCamera()->SampleDirection(bsdfSQ, geomE, bsdfSR);

			// Construct initial ray
			Ray ray;
			ray.o = geomE.p;
			ray.d = bsdfSR.wo;
			ray.minT = Math::Float(0);
			ray.maxT = Math::Constants::Inf();

			// Evaluate importance
			auto We =
				scene.MainCamera()->EvaluatePosition(geomE) *
				scene.MainCamera()->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geomE);

			Math::Vec3 L;
			Math::Vec3 throughput = We / bsdfSR.pdf.v / pdfP.v; // = 1 !!
			int depth = 0;

			while (true)
			{
				// Check intersection
				Intersection isect;
				if (!scene.Intersect(ray, isect))
				{
					break;
				}
					
				const auto* light = isect.primitive->light;
				if (light)
				{
					// Evaluate Le
					GeneralizedBSDFEvaluateQuery bsdfEQ;
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::LightDirection;
					bsdfEQ.wo = -ray.d;
					auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
					auto LeP = light->EvaluatePosition(isect.geom);

					//LM_LOG_DEBUG("T : " + std::to_string(throughput.x) + " " + std::to_string(throughput.y) + " " + std::to_string(throughput.z));
					//LM_LOG_DEBUG("D : " + std::to_string(LeD.x) + " " + std::to_string(LeD.y) + " " + std::to_string(LeD.z));
					//LM_LOG_DEBUG("P : " + std::to_string(LeP.x) + " " + std::to_string(LeP.y) + " " + std::to_string(LeP.z));

					L += throughput * LeD * LeP;
				}

				// --------------------------------------------------------------------------------

				// Sample BSDF
				GeneralizedBSDFSampleQuery bsdfSQ;
				bsdfSQ.sample = rng->NextVec2();
				bsdfSQ.type = GeneralizedBSDFType::AllBSDF;
				bsdfSQ.transportDir = TransportDirection::EL;
				bsdfSQ.wi = -ray.d;
					
				GeneralizedBSDFSampleResult bsdfSR;
				if (!isect.primitive->bsdf->SampleDirection(bsdfSQ, isect.geom, bsdfSR))
				{
					break;
				}

				auto bsdf = isect.primitive->bsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), isect.geom);
				if (Math::IsZero(bsdf))
				{
					break;
				}
					
				// Update throughput
				LM_ASSERT(bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				throughput *= bsdf / bsdfSR.pdf.v;

				// Setup next ray
				ray.d = bsdfSR.wo;
				ray.o = isect.geom.p;
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// --------------------------------------------------------------------------------

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

			film->AccumulateContribution(rasterPos, L * Math::Float(film->Width() * film->Height()) / Math::Float(numSamples));
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(f.get());
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

bool PathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
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
