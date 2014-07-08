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
#include <lightmetrica/pssmlt.splat.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/random.h>
#include <lightmetrica/align.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/align.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

#if 0

/*!
	Per-thread data.
	Contains data associated with a thread.
*/
struct BPTPSSMLTThreadContext : public SIMDAlignedType
{
	
	std::unique_ptr<Random> rng;						//!< Random number generator
	std::unique_ptr<Film> film;							//!< Film
	std::unique_ptr<PSSMLTPrimarySample> sampler;		//!< Kelemen's lazy sampler
	PSSMLTSplats records[2];							//!< Path sample records (current or proposed)
	int current;										//!< Index of current record

	std::unique_ptr<BPTPathVertexPool> pool;			//!< Memory pool for path vertices
	BPTSubpath lightSubpath;							//!< Light subpath
	BPTSubpath eyeSubpath;								//!< Eye subpath

	BPTPSSMLTThreadContext(Random* rng, Film* film, PSSMLTPrimarySample* sampler)
		: rng(rng)
		, film(film)
		, sampler(sampler)
		, pool(new BPTPathVertexPool)
		, lightSubpath(TransportDirection::LE)
		, eyeSubpath(TransportDirection::EL)
	{

	}

	BPTPSSMLTThreadContext(BPTPSSMLTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, sampler(std::move(context.sampler))
		, pool(new BPTPathVertexPool)
		, lightSubpath(TransportDirection::LE)
		, eyeSubpath(TransportDirection::EL)
	{

	}

	PSSMLTSplats& CurentRecord() { return records[current]; }
	PSSMLTSplats& ProposedRecord() { return records[1-current]; }

};

/*!
	Primary sample space Metropolis light transport renderer (BPT version).
	PSSMLT implementation with BPT as an underlying light path sampler.
	Reference:
		Kelemen, C., Szirmay-Kalos, L., Antal, G., and Csonka, F.,
		A simple and robust mutation strategy for the metropolis light transport algorithm,
		In Computer Graphics Forum. pp. 531–540, 2002.
*/
class BPTPSSMLTRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pssmlt.bpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& scene );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, BPTPSSMLTThreadContext& context) const;
	void SampleAndEvaluateBPTPaths(const Scene& scene, PSSMLTSampler& sampler, BPTPathVertexPool& pool, BPTSubpath& lightSubpath, BPTSubpath& eyeSubpath, PSSMLTSplats& splats) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;				// Number of sample mutations
	int rrDepth;						// Depth of beginning RR
	int numThreads;						// Number of threads
	long long samplesPerBlock;			// Samples to be processed per block
	std::string rngType;				// Type of random number generator

	long long numSeedSamples;			// Number of seed samples
	Math::Float largeStepProb;			// Large step mutation probability
	Math::Float kernelSizeS1;			// Minimum kernel size
	Math::Float kernelSizeS2;			// Maximum kernel size

private:

	Math::Float normFactor;											// Normalization factor
	std::unique_ptr<Random> rng;
	std::unique_ptr<PSSMLTRestorableSampler> restorableSampler;		// Restorable sampler for light path generation
	std::vector<PSSMLTPathSeed> seeds;								// Path seeds for each thread

};

bool BPTPSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}

	omp_set_num_threads(numThreads);

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

	node.ChildValueOrDefault("num_seed_samples", 1LL, numSeedSamples);
	node.ChildValueOrDefault("large_step_prob", Math::Float(0.1), largeStepProb);
	node.ChildValueOrDefault("kernel_size_s1", Math::Float(1.0 / 1024.0), kernelSizeS1);
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS2);

	return true;
}

bool BPTPSSMLTRenderer::Preprocess( const Scene& scene )
{
	signal_ReportProgress(0, false);

	// Initialize sampler
	rng.reset(ComponentFactory::Create<Random>(rngType));
	restorableSampler.reset(new PSSMLTRestorableSampler(rng.get(), static_cast<int>(std::time(nullptr))));

	// Take #numSeedSamples samples with BPT and generate seeds for each thread
	// This process must be done in a single thread in order to keep consistency of sample indexes
	PSSMLTSplats splats;
	Math::Float sumI(0);
	std::vector<PSSMLTPathSeed> candidates;

	BPTPathVertexPool pool;
	BPTSubpath lightSubpath(TransportDirection::LE);
	BPTSubpath eyeSubpath(TransportDirection::EL);

	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index
		int index = restorableSampler->Index();

		// Sample light paths
		// We note that BPT might generate multiple light paths
		SampleAndEvaluateBPTPaths(scene, *restorableSampler, pool, lightSubpath, eyeSubpath, splats);

		// Calculate sum of luminance (TODO : or max?)
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
		double u = restorableSampler->Next();
		int idx =
			Math::Clamp(
			static_cast<int>(std::upper_bound(cdf.begin(), cdf.end(), u) - cdf.begin()) - 1,
			0, static_cast<int>(cdf.size()) - 1);

		seeds.push_back(candidates[idx]);
	}

	signal_ReportProgress(1, true);

	return true;
}

bool BPTPSSMLTRenderer::Render( const Scene& scene )
{
	// # Initialize thread context
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::vector<std::unique_ptr<BPTPSSMLTThreadContext>> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		// Add entry
		contexts.emplace_back(new BPTPSSMLTThreadContext(
			ComponentFactory::Create<Random>(rngType), masterFilm->Clone(), new PSSMLTPrimarySample(kernelSizeS1, kernelSizeS2)));

		// Restoring initial state of PSS sampler
		auto& context = contexts.back();
		restorableSampler->SetIndex(seeds[i].index);
		context->sampler->SetRng(restorableSampler->Rng());
		
		// Sample light paths with Kelemen's sampler
		context->current = 0;
		auto& current = context->CurentRecord();
		SampleAndEvaluateBPTPaths(scene, *context->sampler, *context->pool, context->lightSubpath, context->eyeSubpath, current);
		LM_ASSERT(Math::Abs(current.SumI() - seeds[i].I) < Math::Constants::Eps());

		// Restore normal sampler
		context->sampler->SetRng(context->rng.get());
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


void BPTPSSMLTRenderer::ProcessRenderSingleSample( const Scene& scene, BPTPSSMLTThreadContext& context ) const
{
	auto& current  = context.CurentRecord();
	auto& proposed = context.ProposedRecord();

	// Enable large step mutation
	bool enableLargeStep = context.rng->Next() < largeStepProb;
	context.sampler->SetLargeStep(enableLargeStep);

	// Sample and evaluate proposed path
	SampleAndEvaluateBPTPaths(scene, *context.sampler, *context.pool, context.lightSubpath, context.eyeSubpath, proposed);

	// Compute acceptance ratio
	auto currentI  = current.SumI();
	auto proposedI = proposed.SumI();
	auto a = Math::IsZero(currentI) ? Math::Float(1) : Math::Min(Math::Float(1), proposedI / currentI);

	// Determine accept or reject
	if (context.rng->Next() < a)
	{
		context.sampler->Accept();
		context.current = 1 - context.current;
	}
	else
	{
		context.sampler->Reject();
	}

	// Accumulate contribution
	current.AccumulateContributionToFilm(*context.film, (1 - a) * normFactor / currentI);
	current.AccumulateContributionToFilm(*context.film, a * normFactor / proposedI);
}

void BPTPSSMLTRenderer::SampleAndEvaluateBPTPaths( const Scene& scene, PSSMLTSampler& sampler, BPTPathVertexPool& pool, BPTSubpath& lightSubpath, BPTSubpath& eyeSubpath, PSSMLTSplats& splats ) const
{
	// Clear result
	splats.splats.clear();

	// Release and clear paths
	pool.Release();
	lightSubpath.Clear();
	eyeSubpath.Clear();

	// Sample subpaths
	//lightSubpath.Sample(config, scene, *rng, *pool);
	//eyeSubpath.Sample(config, scene, *rng, *pool);


}

LM_COMPONENT_REGISTER_IMPL(BPTPSSMLTRenderer, Renderer);

#endif

LM_NAMESPACE_END