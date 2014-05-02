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
#include <lightmetrica/confignode.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Light trace renderer.
	An implementation of light tracing (a.k.a. inverse path tracing).
	Reference:
		Arvo, J., Backward ray tracing, Developments in Ray Tracing,
		Computer Graphics, Proc. of ACM SIGGRAPH 86 Course Notes, 1986.
*/
class LighttraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("lighttrace");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;		// Number of samples
	int rrDepth;				// Depth of beginning RR
	int numThreads;				// Number of threads
	long long samplesPerBlock;	// Samples to be processed per block
	std::string rngType;		// Type of random number generator

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool LighttraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
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
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!ComponentFactory::CheckRegistered<Random>(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

#if LM_EXPERIMENTAL_MODE
	// Experiments
	auto experimentsNode = node.Child("experiments");
	if (!experimentsNode.Empty())
	{
		LM_LOG_INFO("Configuring experiments");
		LM_LOG_INDENTER();

		if (!expts.Configure(experimentsNode, assets))
		{
			LM_LOG_ERROR("Failed to configure experiments");
			return false;
		}

		if (numThreads != 1)
		{
			LM_LOG_WARN("Number of thread must be 1 in experimental mode, forced 'num_threads' to 1");
			numThreads = 1;
		}
	}
#endif

	return true;
}

bool LighttraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Random>> rngs;
	std::vector<std::unique_ptr<Film>> films;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		rngs.emplace_back(ComponentFactory::Create<Random>(rngType));
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

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			SurfaceGeometry geomL;
			Math::PDFEval pdfPL;

			// Sample a position on the light
			auto lightSampleP = rng->NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, geomL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Evaluate positional component of Le
			auto positionalLe = light->EvaluatePosition(geomL);

			// --------------------------------------------------------------------------------

			// Trace light particle and evaluate importance
			auto throughput = positionalLe / pdfPL.v;
			auto currGeom = geomL;
			Math::Vec3 currWi;
			const GeneralizedBSDF* currBsdf = light;
			int depth = 0;

			while (true)
			{
				// Sample a position on camera
				SurfaceGeometry geomE;
				Math::PDFEval pdfPE;
				scene.MainCamera()->SamplePosition(rng->NextVec2(), geomE, pdfPE);

				// Check connectivity between #geomE.p and #currGeom.p
				Ray shadowRay;
				auto ppE = geomE.p - currGeom.p;
				Math::Float ppEL = Math::Length(ppE);
				shadowRay.d = ppE / ppEL;
				shadowRay.o = currGeom.p;
				shadowRay.minT = Math::Constants::Eps();
				shadowRay.maxT = ppEL * (Math::Float(1) - Math::Constants::Eps());

				Intersection shadowIsect;
				if (!scene.Intersect(shadowRay, shadowIsect))
				{
					// Calculate raster position
					Math::Vec2 rasterPos;
					if (scene.MainCamera()->RayToRasterPosition(geomE.p, -shadowRay.d, rasterPos))
					{
						GeneralizedBSDFEvaluateQuery bsdfEQ;

						// fsL
						bsdfEQ.transportDir = TransportDirection::LE;
						bsdfEQ.type = GeneralizedBSDFType::All;
						bsdfEQ.wi = currWi;
						bsdfEQ.wo = shadowRay.d;
						auto fsL = currBsdf->EvaluateDirection(bsdfEQ, currGeom);

						// fsE
						bsdfEQ.transportDir = TransportDirection::EL;
						bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
						bsdfEQ.wo = -shadowRay.d;
						auto fsE = scene.MainCamera()->EvaluateDirection(bsdfEQ, geomE);

						// Geometry term
						auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomE);

						// Positional component of We
						auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

						// Evaluate contribution and accumulate to film
						auto contrb = throughput * fsL * G * fsE * positionalWe / pdfPE.v;
						film->AccumulateContribution(rasterPos, contrb);
					}
				}

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

				// --------------------------------------------------------------------------------

				// Sample generalized BSDF
				GeneralizedBSDFSampleQuery bsdfSQ;
				bsdfSQ.sample = rng->NextVec2();
				bsdfSQ.transportDir = TransportDirection::LE;
				bsdfSQ.type = GeneralizedBSDFType::All;
				bsdfSQ.wi = currWi;

				GeneralizedBSDFSampleResult bsdfSR;
				if (!currBsdf->SampleDirection(bsdfSQ, currGeom, bsdfSR))
				{
					break;
				}

				// Evaluate generalized BSDF
				auto fs = currBsdf->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), currGeom);
				if (Math::IsZero(fs))
				{
					break;
				}

				// Update throughput
				LM_ASSERT(bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				throughput *= fs / bsdfSR.pdf.v;

				// --------------------------------------------------------------------------------

				// Setup next ray
				Ray ray;
				ray.d = bsdfSR.wo;
				ray.o = currGeom.p;
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// Intersection query
				Intersection isect;
				if (!scene.Intersect(ray, isect))
				{
					break;
				}

				// --------------------------------------------------------------------------------

				// Update information
				currGeom = isect.geom;
				currWi = -ray.d;
				currBsdf = isect.primitive->bsdf;
				depth++;
			}

			LM_EXPT_UPDATE_PARAM(expts, "sample", &sample);
			LM_EXPT_NOTIFY(expts, "SampleFinished");
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(*f.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	return true;
}

LM_COMPONENT_REGISTER_IMPL(LighttraceRenderer, Renderer);

LM_NAMESPACE_END