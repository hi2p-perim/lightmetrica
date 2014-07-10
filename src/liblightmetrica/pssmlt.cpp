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
#include <lightmetrica/pssmlt.common.h>
#include <lightmetrica/pssmlt.pathseed.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/pssmlt.splat.h>
#include <lightmetrica/pssmlt.pathsampler.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/align.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/align.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/defaultexperiments.h>
#include <lightmetrica/rewindablesampler.h>
#include <lightmetrica/random.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Per-thread data.
	Contains data associated with a thread.
*/
struct PSSMLTThreadContext : public SIMDAlignedType
{
	
	std::unique_ptr<Sampler> randomSampler;				//!< Ordinary random sampler
	std::unique_ptr<PSSMLTPathSampler> pathSampler;		//!< Path sampler
	std::unique_ptr<Film> film;							//!< Film
	std::unique_ptr<PSSMLTPrimarySampler> sampler;		//!< Kelemen's lazy sampler
	PSSMLTSplats records[2];							//!< Path sample records (current or proposed)
	int current;										//!< Index of current record

#if 0
	// Experimental variables
	Math::Float kernelSizeScale;
	long long mutated;
	long long accepted;
	//long long kernelUpdateCount;
	//Math::Float expectedAcceptanceRatio;
#endif

	PSSMLTThreadContext(Sampler* randomSampler, PSSMLTPathSampler* pathSampler, Film* film)
		: randomSampler(randomSampler)
		, pathSampler(pathSampler)
		, film(film)
		, sampler(ComponentFactory::Create<PSSMLTPrimarySampler>())
		, current(0)
#if 0
		, kernelSizeScale(1)
		, mutated(0)
		, accepted(0)
#endif
	{

	}

	PSSMLTThreadContext(PSSMLTThreadContext&& context)
		: randomSampler(std::move(context.randomSampler))
		, pathSampler(std::move(context.pathSampler))
		, film(std::move(context.film))
		, sampler(std::move(context.sampler))
		, current(0)
#if 0
		, kernelSizeScale(1)
		, mutated(0)
		, accepted(0)
#endif
	{

	}

	PSSMLTSplats& CurentRecord() { return records[current]; }
	PSSMLTSplats& ProposedRecord() { return records[1-current]; }

};

// --------------------------------------------------------------------------------

enum class PSSMLTEstimatorMode
{
	Normal,
	MeanValueSubstitution,
	MeanValueSubstitution_LargeStepMIS
};

/*!
	Primary sample space Metropolis light transport renderer.
	An implementation of primary sample space Metropolis light transport (PSSMLT) algorithm.
	Reference:
		Kelemen, C., Szirmay-Kalos, L., Antal, G., and Csonka, F.,
		A simple and robust mutation strategy for the metropolis light transport algorithm,
		In Computer Graphics Forum. pp. 531–540, 2002.
*/
class PSSMLTRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pssmlt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, PSSMLTThreadContext& context) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;									//!< Number of sample mutations
	int numThreads;											//!< Number of threads
	long long samplesPerBlock;								//!< Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler
	std::unique_ptr<PSSMLTPathSampler> pathSampler;			//!< Path sampler

	PSSMLTEstimatorMode estimatorMode;						//!< Estimator mode
	long long numSeedSamples;								//!< Number of seed samples
	Math::Float largeStepProb;								//!< Large step mutation probability
	Math::Float kernelSizeS1;								//!< Minimum kernel size
	Math::Float kernelSizeS2;								//!< Maximum kernel size

#if 0
	bool adaptiveKernel;
	//long long kernelUpdateCount;
	//Math::Float kernelScaleDelta;
#endif

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;								//!< Experiments manager
#endif

private:

	Math::Float normFactor;									//!< Normalization factor
	std::unique_ptr<RewindableSampler> rewindableSampler;	//!< Rewindable sampler for light path generation
	std::vector<PSSMLTPathSeed> seeds;						//!< Path seeds for each thread

};

bool PSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
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

	// Set number of threads
	omp_set_num_threads(numThreads);

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

	// Path sampler
	auto pathSamplerNode = node.Child("path_sampler");
	if (pathSamplerNode.Empty())
	{
		LM_LOG_ERROR("Missing 'path_sampler' element");
		return false;
	}
	else
	{
		pathSampler.reset(ComponentFactory::Create<PSSMLTPathSampler>(pathSamplerNode.AttributeValue("type")));
		if (!pathSampler->Configure(pathSamplerNode, assets))
		{
			return false;
		}
	}

	// Estimator mode
	auto estimatorModeNode = node.Child("estimator_mode");
	if (estimatorModeNode.Empty())
	{
		estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS;
		LM_LOG_WARN("Missing 'estimator_mode' element. Using default value.");
	}
	else
	{
		if (estimatorModeNode.Value() == "normal")
		{
			estimatorMode = PSSMLTEstimatorMode::Normal;
		}
		else if (estimatorModeNode.Value() == "mvs")
		{
			estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution;
		}
		else if (estimatorModeNode.Value() == "mvs_mis")
		{
			estimatorMode = PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS;
		}
		else
		{
			LM_LOG_ERROR("Invalid estimator mode '" + estimatorModeNode.Value() + "'");
			return false;
		}
	}

	node.ChildValueOrDefault("num_seed_samples", 1LL, numSeedSamples);
	node.ChildValueOrDefault("large_step_prob", Math::Float(0.1), largeStepProb);
	node.ChildValueOrDefault("kernel_size_s1", Math::Float(1.0 / 1024.0), kernelSizeS1);
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS2);

#if 0
	// Experimental params
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		experimentalNode.ChildValueOrDefault("adaptive_kernel", false, adaptiveKernel);
		//experimentalNode.ChildValueOrDefault("kernel_scale_delta", Math::Float(0.01), kernelScaleDelta);
		//experimentalNode.ChildValueOrDefault("kernel_update_count", 1000LL, kernelUpdateCount);
	}
#endif

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

bool PSSMLTRenderer::Preprocess( const Scene& scene )
{
	signal_ReportProgress(0, false);

	// Initialize sampler
	rewindableSampler.reset(ComponentFactory::Create<RewindableSampler>());
	rewindableSampler->Configure(initialSampler->Rng()->Clone());
	rewindableSampler->SetSeed(initialSampler->NextUInt());

	// Take #numSeedSamples path samples and generate seeds for each thread
	// This process must be done in a single thread in order to keep consistency of sample indexes
	PSSMLTSplats splats;
	Math::Float sumI(0);
	std::vector<PSSMLTPathSeed> candidates;
	
	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index
		int index = rewindableSampler->SampleIndex();

		// Sample light paths
		// We note that path sampler might generate multiple light paths
		pathSampler->SampleAndEvaluate(scene, *rewindableSampler, splats);

		// Calculate sum of luminance
		auto I = splats.SumI();
		if (!Math::IsZero(I))
		{
			sumI += I;
			candidates.emplace_back(index, I);
		}

		signal_ReportProgress(static_cast<double>(sample) / numSeedSamples, false);
	}

	// Normalization factor
	normFactor = sumI / Math::Float(numSeedSamples);

	// Create CDF
	std::vector<Math::Float> cdf;
	cdf.push_back(Math::Float(0));
	for (auto& candidate : candidates)
	{
		cdf.push_back(cdf.back() + candidate.I);
	}

	// Normalize
	auto sum = cdf.back();
	for (auto& v : cdf)
	{
		v /= sum;
	}

	// Sample seeds for each thread
	seeds.clear();
	LM_ASSERT(candidates.size() >= numThreads);
	for (int i = 0; i < numThreads; i++)
	{
		double u = rewindableSampler->Next();
		int idx =
			Math::Clamp(
			static_cast<int>(std::upper_bound(cdf.begin(), cdf.end(), u) - cdf.begin()) - 1,
			0, static_cast<int>(cdf.size()) - 1);

		seeds.push_back(candidates[idx]);
	}

	signal_ReportProgress(1, true);

	return true;
}

bool PSSMLTRenderer::Render( const Scene& scene )
{
	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// # Initialize thread context

	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::vector<std::unique_ptr<PSSMLTThreadContext>> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		// Add an entry
		contexts.emplace_back(new PSSMLTThreadContext(initialSampler->Clone(), pathSampler->Clone(), masterFilm->Clone()));
		
		// Configure and set seeds
		auto& context = contexts.back();
		context->sampler->Configure(initialSampler->Rng()->Clone(), kernelSizeS1, kernelSizeS2);
		context->sampler->SetSeed(initialSampler->NextUInt());
		context->randomSampler->SetSeed(initialSampler->NextUInt());

		// Restore state of the seed path
		rewindableSampler->Rewind(seeds[i].index);
		context->sampler->BeginRestore(*rewindableSampler);
		pathSampler->SampleAndEvaluate(scene, *context->sampler, context->CurentRecord());
		context->sampler->EndRestore();
		LM_ASSERT(Math::Abs(context->CurentRecord().SumI() - seeds[i].I) < Math::Constants::Eps());
	}

	// --------------------------------------------------------------------------------

	// # Rendering process

	std::atomic<long long> processedBlocks(0);								// Number of processes blocks
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;	// Number of blocks
	signal_ReportProgress(0, false);

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& context = contexts[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		LM_EXPT_UPDATE_PARAM(expts, "film", context->film.get());
		LM_EXPT_UPDATE_PARAM(expts, "pssmlt_primary_sample", context->sampler.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			ProcessRenderSingleSample(scene, *context);

			LM_EXPT_UPDATE_PARAM(expts, "sample", &sample);
#if 0
			LM_EXPT_UPDATE_PARAM(expts, "pssmlt_path_length", &current.depth);
			LM_EXPT_UPDATE_PARAM(expts, "pssmlt_acceptance_ratio", &a);
#endif
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
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context->film.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	return true;
}

void PSSMLTRenderer::ProcessRenderSingleSample( const Scene& scene, PSSMLTThreadContext& context ) const
{
	auto& current  = context.CurentRecord();
	auto& proposed = context.ProposedRecord();

	// Enable large step mutation
	bool enableLargeStep = context.randomSampler->Next() < largeStepProb;
	context.sampler->EnableLargeStepMutation(enableLargeStep);

	// Sample and evaluate proposed path
	context.pathSampler->SampleAndEvaluate(scene, *context.sampler, proposed);

	// Compute acceptance ratio
	auto currentI  = current.SumI();
	auto proposedI = proposed.SumI();
	auto a = Math::IsZero(currentI) ? Math::Float(1) : Math::Min(Math::Float(1), proposedI / currentI);

	// Determine accept or reject
	if (context.randomSampler->Next() < a)
	{
		context.sampler->Accept();
		context.current = 1 - context.current;
	}
	else
	{
		context.sampler->Reject();
	}

	// Accumulate contribution
	switch (estimatorMode)
	{
		case PSSMLTEstimatorMode::MeanValueSubstitution:
		{
			if (proposedI > Math::Float(0))
			{
				current.AccumulateContributionToFilm(*context.film, (1 - a) * normFactor / currentI);
				proposed.AccumulateContributionToFilm(*context.film, a * normFactor / proposedI);
			}
			else
			{
				current.AccumulateContributionToFilm(*context.film, normFactor / currentI);
			}
			break;
		}
		case PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS:
		{
			current.AccumulateContributionToFilm(*context.film, (1 - a) / (currentI / normFactor + largeStepProb));
			proposed.AccumulateContributionToFilm(*context.film, (a + (enableLargeStep ? Math::Float(1) : Math::Float(0))) / (proposedI / normFactor + largeStepProb));
			break;
		}
		case PSSMLTEstimatorMode::Normal:
		{
			auto& current = context.CurentRecord();
			current.AccumulateContributionToFilm(*context.film, normFactor / current.SumI());
			break;
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTRenderer, Renderer);

LM_NAMESPACE_END