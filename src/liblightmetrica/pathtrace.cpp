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

	LM_COMPONENT_IMPL_DEF("pt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void SetTerminationMode( RendererTerminationMode mode, double time ) { terminationMode = mode; terminationTime = time; }
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(1, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }
	
private:

	void ProcessRenderSingleSample(const Scene& scene, Sampler& sampler, Film& film) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;
	RendererTerminationMode terminationMode;
	double terminationTime;

private:

	long long numSamples;									// Number of samples
	int rrDepth;											// Depth of beginning RR
	int maxPathVertices;									// Maximum number of light path vertices
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
	node.ChildValueOrDefault("max_path_vertices", -1, maxPathVertices);
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

	// Set number of threads
	omp_set_num_threads(numThreads);

	return true;
}

bool PathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);
	std::atomic<long long> processedSamples(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

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
	bool done = false;
	auto startTime = std::chrono::high_resolution_clock::now();

	while (true)
	{
		#pragma omp parallel for
		for (long long block = 0; block < blocks; block++)
		{
			#pragma omp flush (done)
			if (done)
			{
				continue;
			}

			// --------------------------------------------------------------------------------

			try
			{	
				// Thread ID
				int threadId = omp_get_thread_num();
				auto& sampler = samplers[threadId];
				auto& film = films[threadId];

				// Sample range
				long long sampleBegin = samplesPerBlock * block;
				long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);
				
				processedSamples += sampleEnd - sampleBegin;
				LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

				for (long long sample = sampleBegin; sample < sampleEnd; sample++)
				{
					ProcessRenderSingleSample(scene, *sampler, *film);

					LM_EXPT_UPDATE_PARAM(expts, "sample", &sample);
					LM_EXPT_NOTIFY(expts, "SampleFinished");
				}
			}
			catch (const std::exception& e)
			{
				LM_LOG_ERROR(boost::str(boost::format("EXCEPTION (thread #%d) | %s") % omp_get_thread_num() % e.what()));
				cancel = done = true;
				#pragma omp flush (done)
			}

			// --------------------------------------------------------------------------------

			// Progress report
			processedBlocks++;
			if (terminationMode == RendererTerminationMode::Samples)
			{
				auto progress = static_cast<double>(processedBlocks) / blocks;
				signal_ReportProgress(progress, false);
				LM_EXPT_UPDATE_PARAM(expts, "progress", &progress);
			}
			else if (terminationMode == RendererTerminationMode::Time)
			{
				auto currentTime = std::chrono::high_resolution_clock::now();
				double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count()) / 1000.0;
				if (elapsed > terminationTime)
				{
					done = true;
					#pragma omp flush (done)
				}
				else
				{
					signal_ReportProgress(elapsed / terminationTime, false);
				}
			}
			
			// --------------------------------------------------------------------------------

			LM_EXPT_UPDATE_PARAM(expts, "block", &block);
			LM_EXPT_NOTIFY(expts, "ProgressUpdated");	
		}

		if (done || terminationMode == RendererTerminationMode::Samples)
		{
			break;
		}
	}

	signal_ReportProgress(1, true);

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
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	// --------------------------------------------------------------------------------

	auto finishTime = std::chrono::high_resolution_clock::now();
	double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(finishTime - startTime).count()) / 1000.0;
	LM_LOG_INFO("Rendering completed in " + std::to_string(elapsed) + " seconds");
	LM_LOG_INFO("Processed number of samples : " + std::to_string(processedSamples));

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
	auto We_Estimated = scene.MainCamera()->SampleAndEstimateDirection(bsdfSQ, geomE, bsdfSR);

	// Construct initial ray
	Ray ray;
	ray.o = geomE.p;
	ray.d = bsdfSR.wo;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	Math::Vec3 throughput = We_Estimated;
	Math::Vec3 L;
	int numPathVertices = 1;

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
		
		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = isect.primitive->bsdf->SampleAndEstimateDirection(bsdfSQ, isect.geom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;

		// Setup next ray
		ray.d = bsdfSR.wo;
		ray.o = isect.geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// --------------------------------------------------------------------------------

		if (rrDepth != -1 && numPathVertices >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		numPathVertices++;

		if (maxPathVertices != -1 && numPathVertices >= maxPathVertices)
		{
			break;
		}
	}

	film.AccumulateContribution(rasterPos, L);
}

LM_COMPONENT_REGISTER_IMPL(PathtraceRenderer, Renderer);

LM_NAMESPACE_END
