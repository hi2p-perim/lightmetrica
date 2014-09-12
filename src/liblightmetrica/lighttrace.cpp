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
#include <lightmetrica/logger.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/light.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexperiments.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Light trace renderer.
	An implementation of light tracing (a.k.a. inverse path tracing, particle tracing).
	Reference:
		J. Arvo and D. Kirk, Particle transport and image synthesis,
		Computer Graphics (Procs. of SIGGRAPH 90), 24, 4, pp.63-66, 1990.
*/
class LighttraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("lt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void SetTerminationMode( RendererTerminationMode mode, double time ) {}
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(1, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, Sampler& sampler, Film& film) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;									// Number of samples
	int rrDepth;											// Depth of beginning RR
	int numThreads;											// Number of threads
	long long samplesPerBlock;								// Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;		// Sampler

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
	
	// Sampler
	auto samplerNode = node.Child("sampler");
	auto samplerNodeType = samplerNode.AttributeValue("type");
	if (samplerNodeType != "random")
	{
		LM_LOG_ERROR("Invalid sampler type. This renderer requires 'random' sampler");
		return false;
	}
	initialSampler.reset(ComponentFactory::Create<ConfigurableSampler>(samplerNodeType));
	if (initialSampler == nullptr || !initialSampler->Configure(samplerNode, assets))
	{
		LM_LOG_ERROR("Invalid sampler");
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
	std::vector<std::unique_ptr<Sampler>> samplers;
	std::vector<std::unique_ptr<Film>> films;
	for (int i = 0; i < numThreads; i++)
	{
		samplers.emplace_back(initialSampler->Clone());
		samplers.back()->SetSeed(initialSampler->NextUInt());
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
		auto& sampler = samplers[threadId];
		auto& film = films[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			ProcessRenderSingleSample(scene, *sampler, *film);

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

void LighttraceRenderer::ProcessRenderSingleSample( const Scene& scene, Sampler& sampler, Film& film ) const
{
	SurfaceGeometry geomL;
	Math::PDFEval pdfPL;

	// Sample a position on the light
	auto lightSampleP = sampler.NextVec2();
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
		// Skip if current BSDF is directionally degenerated
		if (!currBsdf->Degenerated())
		{
			// Sample a position on camera
			SurfaceGeometry geomE;
			Math::PDFEval pdfPE;
			scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfPE);

			// Check connectivity between #geomE.p and #currGeom.p
			auto ppE = Math::Normalize(geomE.p - currGeom.p);
			if (RenderUtils::Visible(scene, currGeom.p, geomE.p))
			{
				// Calculate raster position
				Math::Vec2 rasterPos;
				if (scene.MainCamera()->RayToRasterPosition(geomE.p, -ppE, rasterPos))
				{
					GeneralizedBSDFEvaluateQuery bsdfEQ;

					// fsL
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::All;
					bsdfEQ.wi = currWi;
					bsdfEQ.wo = ppE;
					auto fsL = currBsdf->EvaluateDirection(bsdfEQ, currGeom);

					// fsE
					bsdfEQ.transportDir = TransportDirection::EL;
					bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
					bsdfEQ.wo = -ppE;
					auto fsE = scene.MainCamera()->EvaluateDirection(bsdfEQ, geomE);

					// Geometry term
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomE);

					// Positional component of We
					auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

					// Evaluate contribution and accumulate to film
					auto contrb = throughput * fsL * G * fsE * positionalWe / pdfPE.v;
					film.AccumulateContribution(rasterPos, contrb);
				}
			}
		}

		// --------------------------------------------------------------------------------

		if (++depth >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.uComp = sampler.Next();
		bsdfSQ.transportDir = TransportDirection::LE;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = currWi;

		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = currBsdf->SampleAndEstimateDirection(bsdfSQ, currGeom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;

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
}

LM_COMPONENT_REGISTER_IMPL(LighttraceRenderer, Renderer);

LM_NAMESPACE_END