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
struct BPTOptimizedPSSMLTThreadContext : public SIMDAlignedType
{
	
	std::unique_ptr<Sampler> randomSampler;							//!< Ordinary random sampler
	std::unique_ptr<PSSMLTPathSampler> pathSampler;					//!< Path sampler
	std::unique_ptr<Film> film;										//!< Film
	std::unique_ptr<PSSMLTPrimarySampler> lightSubpathSampler;		//!< Kelemen's lazy sampler (for light subpath)
	std::unique_ptr<PSSMLTPrimarySampler> eyeSubpathSampler;		//!< Kelemen's lazy sampler (for eye subpath)
	PSSMLTSplats records[2];										//!< Path sample records (current or proposed)
	int current;													//!< Index of current record

	BPTOptimizedPSSMLTThreadContext(Sampler* randomSampler, PSSMLTPathSampler* pathSampler, Film* film)
		: randomSampler(randomSampler)
		, pathSampler(pathSampler)
		, film(film)
		, lightSubpathSampler(ComponentFactory::Create<PSSMLTPrimarySampler>())
		, eyeSubpathSampler(ComponentFactory::Create<PSSMLTPrimarySampler>())
		, current(0)
	{

	}

	BPTOptimizedPSSMLTThreadContext(BPTOptimizedPSSMLTThreadContext&& context)
		: randomSampler(std::move(context.randomSampler))
		, pathSampler(std::move(context.pathSampler))
		, film(std::move(context.film))
		, lightSubpathSampler(std::move(context.lightSubpathSampler))
		, eyeSubpathSampler(std::move(context.eyeSubpathSampler))
		, current(0)
	{

	}

	PSSMLTSplats& CurrentRecord() { return records[current]; }
	PSSMLTSplats& ProposedRecord() { return records[1-current]; }

};

/*!
	PSSMLT optimized for BPT.
	An implementation of PSSMLT optimized for BPT path sampler
	by separating primary sample space into two parts;
	one for sampling light subpaths and the other for eye subpath.
	Note: some experimental features and estimator modes are omitted.
*/
class BPTOptimizedPSSMLTRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pssmlt.bptopt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, BPTOptimizedPSSMLTThreadContext& context) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;									//!< Number of sample mutations
	int rrDepth;											//!< Depth of beginning RR
	int numThreads;											//!< Number of threads
	long long samplesPerBlock;								//!< Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler
	std::unique_ptr<PSSMLTPathSampler> pathSampler;			//!< Path sampler

	long long numSeedSamples;								//!< Number of seed samples
	Math::Float largeStepProb;								//!< Large step mutation probability
	Math::Float kernelSizeS1;								//!< Minimum kernel size
	Math::Float kernelSizeS2;								//!< Maximum kernel size

private:

	Math::Float normFactor;									//!< Normalization factor
	std::unique_ptr<RewindableSampler> rewindableSampler;	//!< Rewindable sampler for light path generation
	std::vector<PSSMLTPathSeed> seeds;						//!< Path seeds for each thread

};

bool BPTOptimizedPSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
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
	auto pathSamplerType = pathSamplerNode.AttributeValue("type");
	if (pathSamplerType != "bpt")
	{
		LM_LOG_ERROR("Path sampler type must be 'bpt'");
		return false;
	}
	pathSampler.reset(ComponentFactory::Create<PSSMLTPathSampler>(pathSamplerType));
	if (!pathSampler->Configure(pathSamplerNode, assets))
	{
		return false;
	}

	node.ChildValueOrDefault("num_seed_samples", 1LL, numSeedSamples);
	node.ChildValueOrDefault("large_step_prob", Math::Float(0.1), largeStepProb);
	node.ChildValueOrDefault("kernel_size_s1", Math::Float(1.0 / 1024.0), kernelSizeS1);
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS2);

	return true;
}

bool BPTOptimizedPSSMLTRenderer::Preprocess( const Scene& scene )
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
		pathSampler->SampleAndEvaluateBidir(scene, *rewindableSampler, *rewindableSampler, splats, rrDepth, -1);

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

bool BPTOptimizedPSSMLTRenderer::Render( const Scene& scene )
{
	// # Initialize thread context

	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::vector<std::unique_ptr<BPTOptimizedPSSMLTThreadContext>> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		// Add an entry
		contexts.emplace_back(new BPTOptimizedPSSMLTThreadContext(initialSampler->Clone(), pathSampler->Clone(), masterFilm->Clone()));

		// Configure and set seeds
		auto& context = contexts.back();
		context->lightSubpathSampler->Configure(initialSampler->Rng()->Clone(), kernelSizeS1, kernelSizeS2);
		context->lightSubpathSampler->SetSeed(initialSampler->NextUInt());
		context->eyeSubpathSampler->Configure(initialSampler->Rng()->Clone(), kernelSizeS1, kernelSizeS2);
		context->eyeSubpathSampler->SetSeed(initialSampler->NextUInt());
		context->randomSampler->SetSeed(initialSampler->NextUInt());

		// Restore state of the seed path
		rewindableSampler->Rewind(seeds[i].index);
		context->lightSubpathSampler->BeginRestore(*rewindableSampler);
		context->eyeSubpathSampler->BeginRestore(*rewindableSampler);
		pathSampler->SampleAndEvaluateBidir(scene, *context->lightSubpathSampler, *context->eyeSubpathSampler, context->CurrentRecord(), rrDepth, -1);
		context->eyeSubpathSampler->EndRestore();
		context->lightSubpathSampler->EndRestore();

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

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			ProcessRenderSingleSample(scene, *context);
		}

		processedBlocks++;
		auto progress = static_cast<double>(processedBlocks) / blocks;
		signal_ReportProgress(progress, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context->film.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	return true;
}

void BPTOptimizedPSSMLTRenderer::ProcessRenderSingleSample( const Scene& scene, BPTOptimizedPSSMLTThreadContext& context ) const
{
	auto& current  = context.CurrentRecord();
	auto& proposed = context.ProposedRecord();

	// Enable large step mutation
	bool enableLargeStep = context.randomSampler->Next() < largeStepProb;
	context.lightSubpathSampler->EnableLargeStepMutation(enableLargeStep);
	context.eyeSubpathSampler->EnableLargeStepMutation(enableLargeStep);

	// Sample and evaluate proposed path
	context.pathSampler->SampleAndEvaluateBidir(scene, *context.lightSubpathSampler, *context.eyeSubpathSampler, proposed, rrDepth, -1);

	// Compute acceptance ratio
	auto currentI  = current.SumI();
	auto proposedI = proposed.SumI();
	auto a = Math::IsZero(currentI) ? Math::Float(1) : Math::Min(Math::Float(1), proposedI / currentI);

	// Determine accept or reject
	if (context.randomSampler->Next() < a)
	{
		context.lightSubpathSampler->Accept();
		context.eyeSubpathSampler->Accept();
		context.current = 1 - context.current;
	}
	else
	{
		context.lightSubpathSampler->Reject();
		context.eyeSubpathSampler->Reject();
	}

	// Accumulate contribution
	if (proposedI > Math::Float(0))
	{
		current.AccumulateContributionToFilm(*context.film, (1 - a) * normFactor / currentI);
		proposed.AccumulateContributionToFilm(*context.film, a * normFactor / proposedI);
	}
	else
	{
		current.AccumulateContributionToFilm(*context.film, normFactor / currentI);
	}
}

LM_COMPONENT_REGISTER_IMPL(BPTOptimizedPSSMLTRenderer, Renderer);

LM_NAMESPACE_END