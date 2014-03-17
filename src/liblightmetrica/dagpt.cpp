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
#include <lightmetrica/dagpt.h>
#include <lightmetrica/dagpt.graph.h>
#include <lightmetrica/dagpt.pool.h>
#include <lightmetrica/dagpt.sampler.h>
#include <lightmetrica/dagpt.samplerfactory.h>
#include <lightmetrica/dagpt.eval.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/random.h>
#include <lightmetrica/film.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct DAGPTThreadContext
{

	std::unique_ptr<Random> rng;				// Random number generator
	std::unique_ptr<Film> film;					// Film
	std::unique_ptr<DAGPTMemoryPool> pool;		// Memory pool for path vertices

	DAGPTThreadContext(Random* rng, Film* film)
		: rng(rng)
		, film(film)
		, pool(new DAGPTMemoryPool)
	{

	}

	DAGPTThreadContext(DAGPTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, pool(new DAGPTMemoryPool)
	{

	}

};

// --------------------------------------------------------------------------------

class DAGPTRenderer::Impl
{
public:

	Impl(DAGPTRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	DAGPTRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;						// Number of samples
	int rrDepth;								// Depth of beginning RR
	int numThreads;								// Number of threads
	long long samplesPerBlock;					// Samples to be processed per block

	std::unique_ptr<DAGPTLightTransportDAGSampler> dagSampler;			// Sampler for light transport DAG
	std::unique_ptr<DAGPTLightTransportDAGEvaluator> dagEvaluator;		// Evaluator for light transport DAG

};

DAGPTRenderer::Impl::Impl( DAGPTRenderer* self )
	: self(self)
{

}

bool DAGPTRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
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

	// DAG sampler type
	auto dagSamplerNode = node.Child("dag_sampler");
	if (!dagSamplerNode.Empty())
	{
		LM_LOG_ERROR("Missing 'dag_sampler' element");
		return false;
	}

	// Create DAG sampler
	DAGPTLightTransportDAGSamplerFactory factory;
	auto* sampler = factory.Create(dagSamplerNode.AttributeValue("type"));
	if (sampler == nullptr)
	{
		return false;
	}

	dagSampler.reset(sampler);

	return true;
}

bool DAGPTRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<DAGPTThreadContext> contexts;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		contexts.emplace_back(new Random(seed + i), masterFilm->Clone());
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = contexts[threadId].rng;
		auto& film = contexts[threadId].film;
		auto& pool = contexts[threadId].pool;

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		// Light path trees and dag (shared in the same block)
		DAGPTLightTransportDAG dag;

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Release previous contents
			dag.Release(*pool);

			// Sample light transport DAG
			dagSampler->Sample(scene, *rng, *pool, dag);

			// Evaluate contribution
			dagEvaluator->EvaluateContribution(dag, *film);
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(context.film.get());
	}

	return true;
}

// --------------------------------------------------------------------------------

DAGPTRenderer::DAGPTRenderer()
	: p(new Impl(this))
{

}

DAGPTRenderer::~DAGPTRenderer()
{
	LM_SAFE_DELETE(p);
}

bool DAGPTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool DAGPTRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection DAGPTRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END