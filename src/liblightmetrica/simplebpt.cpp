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
#include <lightmetrica/simplebpt.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/random.h>
#include <lightmetrica/light.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/renderutils.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

class SimpleBidirectionalPathtraceRenderer::Impl : public Object
{
public:

	Impl(SimpleBidirectionalPathtraceRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	SimpleBidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block

};

SimpleBidirectionalPathtraceRenderer::Impl::Impl( SimpleBidirectionalPathtraceRenderer* self )
	: self(self)
{

}

bool SimpleBidirectionalPathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	// Check type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid renderer type '" + node.AttributeValue("type") + "'");
		return false;
	}

	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 0, rrDepth);
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

	return true;
}

bool SimpleBidirectionalPathtraceRenderer::Impl::Render( const Scene& scene )
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
		rngs.push_back(std::unique_ptr<Random>(new Random(seed + i)));
		films.push_back(std::unique_ptr<Film>(masterFilm->Clone()));
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
			SurfaceGeometry geomE;
			Math::PDFEval pdfPE;

			// Sample position on the camera
			scene.MainCamera()->SamplePosition(rng->NextVec2(), geomE, pdfPE);
			
			// Evaluate We^{(0)} (positional component of We)
			auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

			// --------------------------------------------------------------------------------

			SurfaceGeometry geomL;
			Math::PDFEval pdfPL;

			// Sample a position on the light
			auto lightSampleP = rng->NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, geomL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Evaluate Le^{(0)} (positional component of Le)
			auto positionalLe = light->EvaluatePosition(geomL);

			// --------------------------------------------------------------------------------

			// Length of eye and light sub-paths respectively
			int pathLength[2] = { 0 };

			// Path throughputs
			Math::Vec3 throughput[2] = { positionalWe / pdfPE.v, positionalLe / pdfPL.v };

			// Current generalized BSDF
			const GeneralizedBSDF* currBsdfs[2] = { scene.MainCamera(), light };

			// Current and previous positions and geometry normals
			SurfaceGeometry currGeom[2] = { geomE, geomL };
			Math::Vec3 currWi[2];

			// Raster position
			Math::Vec2 rasterPos;

			while (true)
			{
				// Check connectivity between #pE and #pL
				Ray shadowRay;
				auto pEpL = currGeom[TransportDirection::LE].p - currGeom[TransportDirection::EL].p;
				auto pEpL_Length = Math::Length(pEpL);
				shadowRay.o = currGeom[TransportDirection::EL].p;
				shadowRay.d = pEpL / pEpL_Length;
				shadowRay.minT = Math::Constants::Eps();
				shadowRay.maxT = pEpL_Length * (Math::Float(1) - Math::Constants::Eps());

				Intersection shadowIsect;
				if (!scene.Intersect(shadowRay, shadowIsect))
				{
					bool visible = true;
					if (pathLength[TransportDirection::EL] == 0)
					{
						// If the length of eye sub-path is zero, compute raster position here
						visible = scene.MainCamera()->RayToRasterPosition(shadowRay.o, shadowRay.d, rasterPos);
					}

					// If #rasterPos is visible in the screen ..
					if (visible)
					{
						GeneralizedBSDFEvaluateQuery bsdfEQ;

						// fsE
						bsdfEQ.transportDir = TransportDirection::EL;
						bsdfEQ.type = GeneralizedBSDFType::All;
						bsdfEQ.wi = currWi[TransportDirection::EL];
						bsdfEQ.wo = shadowRay.d;
						auto fsE = currBsdfs[TransportDirection::EL]->EvaluateDirection(bsdfEQ, currGeom[TransportDirection::EL]);

						// fsL
						bsdfEQ.transportDir = TransportDirection::LE;
						bsdfEQ.type = GeneralizedBSDFType::All;
						bsdfEQ.wi = currWi[TransportDirection::LE];
						bsdfEQ.wo = -shadowRay.d;
						auto fsL = currBsdfs[TransportDirection::LE]->EvaluateDirection(bsdfEQ, currGeom[TransportDirection::LE]);

						// Geometry term
						auto G = RenderUtils::GeneralizedGeometryTerm(currGeom[TransportDirection::EL], currGeom[TransportDirection::LE]);

						// Evaluate contribution and accumulate to film
						auto contrb = throughput[TransportDirection::EL] * fsE * G * fsL * throughput[TransportDirection::LE];
						film->AccumulateContribution(rasterPos, contrb * Math::Float(film->Width() * film->Height()) / Math::Float(numSamples));
					}
				}

				// --------------------------------------------------------------------------------

				// Select which sub-path to be extended
				// This selection might be tricky. TODO : Add some explanation
				int subpath = rng->Next() < Math::Float(0.5) ? TransportDirection::EL : TransportDirection::LE;

				// Decide if the selected -path is actually extended by Russian roulette
				// If the sub-path is not extended, terminate the loop.
				if (pathLength[subpath] >= rrDepth)
				{
					auto p = Math::Min(Math::Float(0.5), Math::Luminance(throughput[subpath]));
					if (rng->Next() > p)
					{
						break;
					}
					else
					{
						throughput[subpath] /= p;
					}
				}

				// --------------------------------------------------------------------------------

				// Sample generalized BSDF
				GeneralizedBSDFSampleQuery bsdfSQ;
				bsdfSQ.sample = rng->NextVec2();
				bsdfSQ.transportDir = (TransportDirection)subpath;
				bsdfSQ.type = GeneralizedBSDFType::All;
				bsdfSQ.wi = currWi[subpath];

				GeneralizedBSDFSampleResult bsdfSR;
				if (!currBsdfs[subpath]->SampleDirection(bsdfSQ, currGeom[subpath], bsdfSR))
				{
					break;
				}
					
				// Evaluate generalized BSDF
				auto fs = currBsdfs[subpath]->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), currGeom[subpath]);
				if (Math::IsZero(fs))
				{
					break;
				}

				// Update throughput
				LM_ASSERT(bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				throughput[subpath] *= fs / bsdfSR.pdf.v;

				// --------------------------------------------------------------------------------
				
				// Setup next ray
				Ray ray;
				ray.d = bsdfSR.wo;
				ray.o = currGeom[subpath].p;
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// Intersection query
				Intersection isect;
				if (!scene.Intersect(ray, isect))
				{
					break;
				}

				// --------------------------------------------------------------------------------

				// Compute raster position if current length of eye sub-path is zero
				if (subpath == TransportDirection::EL && pathLength[TransportDirection::EL] == 0 && !scene.MainCamera()->RayToRasterPosition(ray.o, ray.d, rasterPos))
				{
					break;
				}

				// Update information
				currGeom[subpath] = isect.geom;
				currWi[subpath] = -ray.d;
				currBsdfs[subpath] = isect.primitive->bsdf;
				pathLength[subpath]++;
			}
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

SimpleBidirectionalPathtraceRenderer::SimpleBidirectionalPathtraceRenderer()
	: p(new Impl(this))
{

}

SimpleBidirectionalPathtraceRenderer::~SimpleBidirectionalPathtraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool SimpleBidirectionalPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool SimpleBidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection SimpleBidirectionalPathtraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END