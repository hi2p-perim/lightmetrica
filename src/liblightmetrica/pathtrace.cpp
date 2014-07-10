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
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexperiments.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Path trace renderer.
	An implementation of path tracing.
	Reference:
		J. T. Kajiya, The rendering equation,
		Procs. of the 13th annual conference on Computer graphics and interactive techniques, 1986,
*/
class PathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pathtrace");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(0, true); return true; }
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
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool PathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
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

	// Sampler
	auto samplerNode = node.Child("sampler");
	initialSampler.reset(ComponentFactory::Create<ConfigurableSampler>(samplerNode.AttributeValue("type")));
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

bool PathtraceRenderer::Render( const Scene& scene )
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

	bool cancel = false;

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		try
		{
			if (cancel)
			{
				continue;
			}
			
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
		catch (const std::exception& e)
		{
			LM_LOG_ERROR(boost::str(boost::format("EXCEPTION (thread #%d) | %s") % omp_get_thread_num() % e.what()));
			cancel = true;
		}
	}

	if (cancel)
	{
		LM_LOG_ERROR("Render operation has been canceled");
		return false;
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

void PathtraceRenderer::ProcessRenderSingleSample( const Scene& scene, Sampler& sampler, Film& film ) const
{
	// Raster position
	auto rasterPos = sampler.NextVec2();

	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfP;
	scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfP);

	// Sample ray direction
	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;
	bsdfSQ.sample = rasterPos;
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
#if 1
	auto We_Estimated = scene.MainCamera()->SampleAndEstimateDirection(bsdfSQ, geomE, bsdfSR);
#else
	scene.MainCamera()->SampleDirection(bsdfSQ, geomE, bsdfSR);
#endif

	// Construct initial ray
	Ray ray;
	ray.o = geomE.p;
	ray.d = bsdfSR.wo;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	Math::Vec3 throughput;
#if 1
	throughput = We_Estimated;
#elif 1
	throughput = Math::Vec3(Math::Float(1));
#else
	// Evaluate importance
	auto We =
		scene.MainCamera()->EvaluatePosition(geomE) *
		scene.MainCamera()->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), geomE);
	throughput = We / bsdfSR.pdf.v / pdfP.v; // = 1 !!
#endif
			
	Math::Vec3 L;
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
			L += throughput * LeD * LeP;
		}

		// --------------------------------------------------------------------------------

		// Sample BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.uComp = sampler.Next();
		bsdfSQ.type = GeneralizedBSDFType::AllBSDF;
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.wi = -ray.d;
		
#if 1
		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = isect.primitive->bsdf->SampleAndEstimateDirection(bsdfSQ, isect.geom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;
#else
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
#endif

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
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}
	}

	film.AccumulateContribution(rasterPos, L);
}

LM_COMPONENT_REGISTER_IMPL(PathtraceRenderer, Renderer);

LM_NAMESPACE_END
