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
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexpts.h>
#include <lightmetrica/renderutils.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Path tracing with direct light sampling.
	Implements path tracing with direct light sampling (a.k.a. next event estimation).
	In this implementation,
	E{D,S}D+L paths are sampled by direct light sampling and
	E{D,S}*S+L paths are sampled by BSDF sampling.
*/
class DirectPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pathtrace.direct");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene ) { signal_ReportProgress(0, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, Random& rng, Film& film) const;

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

bool DirectPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
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

bool DirectPathtraceRenderer::Render( const Scene& scene )
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
			ProcessRenderSingleSample(scene, *rng, *film);

			LM_EXPT_UPDATE_PARAM(expts, "sample", &sample);
			LM_EXPT_NOTIFY(expts, "SampleFinished");
		}

		processedBlocks++;
		auto progress = static_cast<double>(processedBlocks) / blocks;
		signal_ReportProgress(progress, processedBlocks == blocks);

		LM_EXPT_UPDATE_PARAM(expts, "block", &block);
		LM_EXPT_UPDATE_PARAM(expts, "progress", &progress);
		LM_EXPT_NOTIFY(expts, "ProgressUpdated");
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

void DirectPathtraceRenderer::ProcessRenderSingleSample( const Scene& scene, Random& rng, Film& film ) const
{
	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(rng.NextVec2(), geomE, pdfPE);

	// Evaluate positional component of We
	auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

	// Trace ray from camera
	auto throughput = positionalWe / pdfPE.v;
	auto currGeom = geomE;
	Math::Vec3 currWi;
	const GeneralizedBSDF* currBsdf = scene.MainCamera();
	int depth = 0;
	Math::Vec2 rasterPos;

	while (true)
	{
		// Skip if current BSDF is directionally degenerated
		if (!currBsdf->Degenerated())
		{
			// Sample a position on light
			SurfaceGeometry geomL;
			Math::PDFEval pdfPL;
			auto lightSampleP = rng.NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, geomL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Check connectivity between #currGeom.p and #geomL.p  
			auto ppL = Math::Normalize(geomL.p - currGeom.p);
			if (RenderUtils::Visible(scene, currGeom.p, geomL.p))
			{
				// Calculate raster position if the depth is zero
				bool visible = true;
				if (depth == 0)
				{
					visible = scene.MainCamera()->RayToRasterPosition(currGeom.p, ppL, rasterPos);
				}

				if (visible)
				{
					GeneralizedBSDFEvaluateQuery bsdfEQ;

					// fsE
					bsdfEQ.transportDir = TransportDirection::EL;
					bsdfEQ.type = GeneralizedBSDFType::All;
					bsdfEQ.wi = currWi;
					bsdfEQ.wo = ppL;
					auto fsE = currBsdf->EvaluateDirection(bsdfEQ, currGeom);

					// fsL
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::LightDirection;
					bsdfEQ.wo = -ppL;
					auto fsL = light->EvaluateDirection(bsdfEQ, geomL);

					// Geometry term
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomL);

					// Positional component of Le
					auto positionalLe = light->EvaluatePosition(geomL);

					// Evaluate contribution and accumulate to film
					auto contrb = throughput * fsE * G * fsL * positionalLe / pdfPL.v;
					film.AccumulateContribution(rasterPos, contrb);
				}
			}
		}

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

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.uComp = rng.Next();
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = currWi;

#if 1
		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = currBsdf->SampleAndEstimateDirection(bsdfSQ, currGeom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;
#else
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
#endif

		// Calculate raster position if the depth is one
		if (depth == 1)
		{
			if (!scene.MainCamera()->RayToRasterPosition(currGeom.p, bsdfSR.wo, rasterPos))
			{
				// Should not be here
				LM_ASSERT(false);
				break;
			}
		}

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

		// Intersected point is light
		{
			const auto* light = isect.primitive->light;
			if (light != nullptr)
			{
				// Previous BSDF is specular 
				if ((bsdfSR.sampledType & GeneralizedBSDFType::Specular) > 0)
				{
					// Evaluate Le
					GeneralizedBSDFEvaluateQuery bsdfEQ;
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::LightDirection;
					bsdfEQ.wo = -ray.d;
					auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
					auto LeP = light->EvaluatePosition(isect.geom);
					film.AccumulateContribution(rasterPos, throughput * LeD * LeP);
				}
			}
		}

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
		depth++;
	}
}

LM_COMPONENT_REGISTER_IMPL(DirectPathtraceRenderer, Renderer);

LM_NAMESPACE_END