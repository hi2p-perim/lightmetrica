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
#include <lightmetrica/pssmlt.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/hdrfilm.h>
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
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*
	PSSMLT sampler.
	Sampler interface for uniform random number generator
	used in the PSSMLT implementation.
*/
class PSSMLTSampler
{
public:

	virtual ~PSSMLTSampler() {}
	virtual Math::Float Next() = 0;
	virtual Random* Rng() = 0;
	Math::Vec2 NextVec2() { return Math::Vec2(Next(), Next()); }

};

/*
	Restorable sampler.
	An implementation of uniform random number generator
	which can be restored in the specified index.
*/
class PSSMLTRestorableSampler : public PSSMLTSampler
{
public:

	PSSMLTRestorableSampler(unsigned int seed)
		: initialSeed(seed)
		, rng(new Random(seed))
		, currentIndex(0)
	{

	}

	PSSMLTRestorableSampler(const PSSMLTRestorableSampler& sampler)
		: initialSeed(sampler.initialSeed)
		, rng(new Random(sampler.initialSeed))
		, currentIndex(0)
	{

	}

	Math::Float Next()
	{
		currentIndex++;
		return rng->Next();
	}

	void SetIndex(int index)
	{
		// Reset the initial seed
		currentIndex = 0;
		rng->SetSeed(initialSeed);

		// Generate samples until the given index
		while (currentIndex < index)
		{
			// Discard the value
			currentIndex++;
			rng->Next();
		}
	}

	int Index() { return currentIndex; }
	Random* Rng() { return rng.get(); }

private:

	unsigned int initialSeed;
	std::unique_ptr<Random> rng;

	// Number of generated samples
	int currentIndex;

};

/*
	Kelemen's lazy sampler.
	For details, see the original paper.
*/
class PSSMLTLazySampler : public PSSMLTSampler
{
public:

	struct Sample
	{
		Sample(const Math::Float& value)
			: value(value)
			, modify(0)
		{}

		Math::Float value;		// Sample value
		long long modify;		// Last modified time
	};

public:

	PSSMLTLazySampler(const Math::Float& s1, const Math::Float& s2)
		: s1(s1)
		, s2(s2)
	{
		logRatio = -std::log(s2 / s1);
		time = 0;
		largeStepTime = 0;
		largeStep = false;
		currentIndex = 0;
	}

	void Accept()
	{
		if (largeStep)
		{
			// Update large step time
			largeStepTime = time;
		}

		time++;
		prevSamples.clear();
		currentIndex = 0;
	}

	void Reject()
	{
		// Restore samples
		for (auto& v : prevSamples)
		{
			u[std::get<0>(v)] = std::get<1>(v);
		}

		prevSamples.clear();
		currentIndex = 0;
	}

	Math::Float Next()
	{
		return PrimarySample(currentIndex++);
	}

	void SetLargeStep(bool largeStep) { this->largeStep = largeStep; }
	bool LargeStep() { return largeStep; }

	void SetRng(Random* rng) { this->rng = rng; }
	Random* Rng() { return rng; }

private:

	Math::Float PrimarySample(int i)
	{
		// Not sampled yet
		while (i >= (int)u.size())
		{
			u.push_back(Sample(rng->Next()));
		}

		// If the modified time of the requested sample is not updated
		// it requires the lazy evaluation of mutations.
		if (u[i].modify < time)
		{
			if (largeStep)
			{
				// Large step case

				// Save sample in order to restore previous state
				prevSamples.push_back(std::make_tuple(i, u[i]));

				// Update the modified time and value
				u[i].modify = time;
				u[i].value = rng->Next();
			}
			else
			{
				// Small step case

				// If the modified time is not updated since the last accepted
				// large step mutation, then update sample to the state.
				// Note that there is no need to go back before largeStepTime
				// because these samples are independent of the sample on largeStepTime.
				if (u[i].modify < largeStepTime)
				{
					u[i].modify = largeStepTime;
					u[i].value = rng->Next();
				}

				// Lazy evaluation of Mutate
				while (u[i].modify < time - 1)
				{
					u[i].value = Mutate(u[i].value);
					u[i].modify++;
				}

				// Save state
				prevSamples.push_back(std::make_tuple(i, u[i]));

				// Update the modified time and value
				u[i].value = Mutate(u[i].value);
				u[i].modify++;
			}
		}

		return u[i].value;
	}

	Math::Float Mutate(const Math::Float& value)
	{
		Math::Float u = rng->Next();
		bool positive = u < Math::Float(0.5);

		// Convert to [0, 1]
		u = positive ? u * Math::Float(2) : Math::Float(2) * (u - Math::Float(0.5)); 

		Math::Float dv = s2 * std::exp(logRatio * u);

		Math::Float result = value;
		if (positive)
		{
			result += dv;
			if (result > Math::Float(1)) result -= Math::Float(1);
		}
		else
		{
			result -= dv;
			if (result < Math::Float(0)) result += Math::Float(1);
		}

		return result;
	}

private:

	Math::Float s1, s2;
	Math::Float logRatio;

	Random* rng;				// Random number generator (not managed here)

	long long time;				// Number of accepted mutations
	long long largeStepTime;	// Time of the last accepted large step
	bool largeStep;				// Indicates the next mutation is the large step

	int currentIndex;
	std::vector<Sample> u;
	std::vector<std::tuple<int, Sample>> prevSamples;

};

// --------------------------------------------------------------------------------

/*
	Light path seed.
	Required data to generate a seed light path.
*/
struct PSSMLTPathSeed
{

	int index;			// Sample index of restorable sampler
	Math::Float I;		// Luminance of the sampled light path (used for debugging)

	PSSMLTPathSeed()
	{

	}

	PSSMLTPathSeed(int index, const Math::Float& I)
		: index(index)
		, I(I)
	{

	}

};

/*
	Light path sample record.
	Contains some information of sampled light paths.
*/
struct PSSMLTPathSampleRecord : public Object
{

	Math::Vec2 rasterPos;		// Raster position
	Math::Vec3 L;				// Sampled radiance

};

// --------------------------------------------------------------------------------

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct PSSMLTThreadContext
{
	
	std::unique_ptr<Random> rng;						// Random number generator
	std::unique_ptr<Film> film;							// Film
	std::unique_ptr<PSSMLTLazySampler> sampler;			// Kelemen's lazy sampler
	PSSMLTPathSampleRecord records[2];					// Path sample records (current or proposed)
	int current;										// Index of current record

	PSSMLTThreadContext(Random* rng, Film* film, PSSMLTLazySampler* sampler)
		: rng(rng)
		, film(film)
		, sampler(sampler)
	{

	}

	PSSMLTThreadContext(PSSMLTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, sampler(std::move(context.sampler))
	{

	}

};

// --------------------------------------------------------------------------------

enum class PSSMLTEstimatorMode
{
	Normal,
	MeanValueSubstitution,
	MeanValueSubstitution_LargeStepMIS
};

class PSSMLTRenderer::Impl
{
public:

	Impl(PSSMLTRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	void GenerateAndSampleSeeds(const Scene& scene, PSSMLTRestorableSampler& restorableSampler, Math::Float& B, std::vector<PSSMLTPathSeed>& seeds) const;
	void SampleAndEvaluatePath(const Scene& scene, PSSMLTSampler& sampler, Math::Vec3& L, Math::Vec2& rasterPos) const;

private:

	PSSMLTRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;						// Number of sample mutations
	int rrDepth;								// Depth of beginning RR
	int numThreads;								// Number of threads
	long long samplesPerBlock;					// Samples to be processed per block

	PSSMLTEstimatorMode estimatorMode;			// Estimator mode
	long long numSeedSamples;					// Number of seed samples
	Math::Float largeStepProb;					// Large step mutation probability
	Math::Float kernelSizeS1;					// Minimum kernel size
	Math::Float kernelSizeS2;					// Maximum kernel size

};

PSSMLTRenderer::Impl::Impl( PSSMLTRenderer* self )
	: self(self)
{
	
}

bool PSSMLTRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
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
	node.ChildValueOrDefault("kernel_size_s2", Math::Float(1.0 / 64.0), kernelSizeS1);

	return true;
}

bool PSSMLTRenderer::Impl::Render( const Scene& scene )
{
	// Set number of threads
	omp_set_num_threads(numThreads);

	// --------------------------------------------------------------------------------

	// Preprocess
	Math::Float B;
	std::vector<PSSMLTPathSeed> seeds;
	int seed = static_cast<int>(std::time(nullptr));
	PSSMLTRestorableSampler restorableSampler(seed++);
	GenerateAndSampleSeeds(scene, restorableSampler, B, seeds);

	// --------------------------------------------------------------------------------

	// Setup thread context
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::vector<PSSMLTThreadContext> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		// Add a entry to the context
		contexts.emplace_back(
			new Random(seed++),
			masterFilm->Clone(),
			new PSSMLTLazySampler(kernelSizeS1, kernelSizeS2));
		
		// Setup initial state of the primary sample space sampler
		// by restoring the state of the initial path.

		// Set the sample index to the restorable sampler
		// and prepare for restoring the sampled light path.
		auto& context = contexts.back();
		restorableSampler.SetIndex(seeds[i].index);
		context.sampler->SetRng(restorableSampler.Rng());

		// Sample a seed light path and initialize the state of Kelemen's lazy sampler
		context.current = 0;
		auto& current = context.records[context.current];
		SampleAndEvaluatePath(scene, *context.sampler, current.L, current.rasterPos);
		LM_ASSERT(Math::Abs(seeds[i].I - Math::Luminance(current.L)) < Math::Constants::Eps());

		// Get back to the normal generator
		context.sampler->SetRng(context.rng.get());
	}

	// --------------------------------------------------------------------------------

	// Rendering
	std::atomic<long long> processedBlocks = 0;								// # of processes blocks
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;	// # of blocks
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
			auto& current = context.records[context.current];
			auto& proposed = context.records[1-context.current];

			// Enable flag for large step mutation
			// Note that the probability is not related to the state
			bool enableLargeStep = context.rng->Next() < largeStepProb;
			context.sampler->SetLargeStep(enableLargeStep);

			// Sample and evaluate proposed path
			SampleAndEvaluatePath(scene, *context.sampler, proposed.L, proposed.rasterPos);

			// Compute acceptance ratio
			auto currentI = Math::Luminance(current.L);
			auto proposedI = Math::Luminance(proposed.L);
			auto a = currentI > Math::Float(0)
				? Math::Min(Math::Float(1), proposedI / currentI)
				: Math::Float(1);

			if (context.rng->Next() < a)
			{
				// Accepted
				context.sampler->Accept();
				context.current = 1 - context.current;
			}
			else
			{
				// Rejected
				context.sampler->Reject();
			}

			// Accumulate contribution
			if (estimatorMode == PSSMLTEstimatorMode::MeanValueSubstitution)
			{
				context.film->AccumulateContribution(current.rasterPos, current.L * (1 - a) * B / currentI);
				context.film->AccumulateContribution(proposed.rasterPos, proposed.L * a * B / proposedI);
			}
			else if (estimatorMode == PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS)
			{
				context.film->AccumulateContribution(current.rasterPos, current.L * (1 - a) / (currentI / B + largeStepProb));
				context.film->AccumulateContribution(proposed.rasterPos, proposed.L * (a + (enableLargeStep ? Math::Float(1) : Math::Float(0))) / (proposedI / B + largeStepProb));
			}
			else if (estimatorMode == PSSMLTEstimatorMode::Normal)
			{
				auto& current = context.records[context.current];
				auto currentI = Math::Luminance(current.L);
				context.film->AccumulateContribution(current.rasterPos, current.L * B / currentI);
			}
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

void PSSMLTRenderer::Impl::GenerateAndSampleSeeds( const Scene& scene, PSSMLTRestorableSampler& restorableSampler, Math::Float& B, std::vector<PSSMLTPathSeed>& seeds ) const
{
	// Generate candidates for seeds
	std::vector<PSSMLTPathSeed> candidates;
	Math::Float sumI(0);

	signal_ReportProgress(0, false);

	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index of #restorableSampler
		int index = restorableSampler.Index();

		// Sample a light path and evaluate radiance
		Math::Vec3 L;
		Math::Vec2 _;
		SampleAndEvaluatePath(scene, restorableSampler, L, _);

		if (!Math::IsZero(L))
		{
			auto I = Math::Luminance(L);
			sumI += I;
			candidates.emplace_back(index, I);
		}

		signal_ReportProgress(static_cast<double>(sample) / numSeedSamples, false);
	}
	
	// Compute #B
	B = sumI / Math::Float(numSeedSamples);

	// Sample seeds according to I

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
		double u = restorableSampler.Next();
		int idx =
			Math::Clamp(
				static_cast<int>(std::upper_bound(cdf.begin(), cdf.end(), u) - cdf.begin()) - 1,
				0, static_cast<int>(cdf.size()) - 1);

		seeds.push_back(candidates[idx]);
	}
}

void PSSMLTRenderer::Impl::SampleAndEvaluatePath( const Scene& scene, PSSMLTSampler& sampler, Math::Vec3& L, Math::Vec2& rasterPos ) const
{
	rasterPos = sampler.NextVec2();
	L = Math::Vec3(1);
}

// --------------------------------------------------------------------------------

PSSMLTRenderer::PSSMLTRenderer()
	: p(new Impl(this))
{

}

PSSMLTRenderer::~PSSMLTRenderer()
{
	LM_SAFE_DELETE(p);
}

bool PSSMLTRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool PSSMLTRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection PSSMLTRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END