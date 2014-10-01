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
#include <lightmetrica/renderproc.h>
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
#include <lightmetrica/math.distribution.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

enum class PSSMLTEstimatorMode
{
	Normal,
	MeanValueSubstitution,
	MeanValueSubstitution_LargeStepMIS
};

// --------------------------------------------------------------------------------

/*!
	Primary sample space Metropolis light transport renderer.
	An implementation of primary sample space Metropolis light transport (PSSMLT) algorithm.
	Reference:
		Kelemen, C., Szirmay-Kalos, L., Antal, G., and Csonka, F.,
		A simple and robust mutation strategy for the metropolis light transport algorithm,
		In Computer Graphics Forum. pp. 531–540, 2002.
*/
class PSSMLTRenderer final : public Renderer
{
private:

	friend class PSSMLTRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("pssmlt");

public:

	virtual std::string Type() const override { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene, const RenderProcessScheduler& sched) override;
	virtual bool Preprocess(const Scene& scene, const RenderProcessScheduler& sched) override;
	virtual bool Postprocess(const Scene& scene, const RenderProcessScheduler& sched) const override { return true; }
	virtual RenderProcess* CreateRenderProcess(const Scene& scene, int threadID, int numThreads) override;
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void(double, bool)>& func) override { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

private:

	int rrDepth;											//!< Depth of beginning RR
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler

private:

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;								//!< Experiments manager
#endif

private:

	std::unique_ptr<PSSMLTPathSampler> pathSampler;			//!< Path sampler
	PSSMLTEstimatorMode estimatorMode;						//!< Estimator mode
	long long numSeedSamples;								//!< Number of seed samples
	Math::Float largeStepProb;								//!< Large step mutation probability
	Math::Float kernelSizeS1;								//!< Minimum kernel size
	Math::Float kernelSizeS2;								//!< Maximum kernel size

private:

	Math::Float normFactor;									//!< Normalization factor
	std::unique_ptr<RewindableSampler> rewindableSampler;	//!< Rewindable sampler for light path generation
	std::vector<PSSMLTPathSeed> seedCandidates;				//!< Candidates of seeds for each thread
	Math::DiscreteDistribution1D seedCandidateDist;			//!< Distribution for seed selection

};

// --------------------------------------------------------------------------------

/*!
	Render process for PSSMLTRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class PSSMLTRenderer_RenderProcess : public SamplingBasedRenderProcess
{
public:

	PSSMLTRenderer_RenderProcess(const PSSMLTRenderer& renderer, Sampler* randomSampler, PSSMLTPathSampler* pathSampler, Film* film)
		: renderer(renderer)
		, randomSampler(randomSampler)
		, pathSampler(pathSampler)
		, film(film)
		, sampler(ComponentFactory::Create<PSSMLTPrimarySampler>())
		, currentIdx(0)
	{

	}

private:

	LM_DISABLE_COPY_AND_MOVE(PSSMLTRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

public:

	bool Configure(const Scene& scene, const PSSMLTPathSeed& seed);

private:

	PSSMLTSplats& Current() { return records[currentIdx]; }
	PSSMLTSplats& Proposed() { return records[1-currentIdx]; }

private:

	const PSSMLTRenderer& renderer;
	std::unique_ptr<Sampler> randomSampler;					//!< Ordinary random sampler
	std::unique_ptr<PSSMLTPathSampler> pathSampler;			//!< Path sampler
	std::unique_ptr<Film> film;								//!< Film
	std::unique_ptr<PSSMLTPrimarySampler> sampler;			//!< Kelemen's lazy sampler
	PSSMLTSplats records[2];								//!< Path sample records (current or proposed)
	int currentIdx;											//!< Index of current record

};

// --------------------------------------------------------------------------------

bool PSSMLTRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene, const RenderProcessScheduler& sched)
{
	// Load parameters
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);

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
	}
#endif

	return true;
}

bool PSSMLTRenderer::Preprocess( const Scene& scene, const RenderProcessScheduler& sched )
{
	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// # Initialize sampler
	rewindableSampler.reset(ComponentFactory::Create<RewindableSampler>());
	rewindableSampler->Configure(initialSampler->Rng()->Clone());
	rewindableSampler->SetSeed(initialSampler->NextUInt());

	// --------------------------------------------------------------------------------

	// # Sample candidates for seeds

	// Take #numSeedSamples path samples and generate seeds for each thread
	// This process must be done in a single thread in order to keep consistency of sample indexes

	PSSMLTSplats splats;
	Math::Float sumI(0);

	for (long long sample = 0; sample < numSeedSamples; sample++)
	{
		// Current sample index
		int index = rewindableSampler->SampleIndex();

		// Sample light paths
		// We note that path sampler might generate multiple light paths
		pathSampler->SampleAndEvaluate(scene, *rewindableSampler, splats, rrDepth, -1);

		// Calculate sum of luminance
		auto I = splats.SumI();
		if (!Math::IsZero(I))
		{
			sumI += I;
			seedCandidates.emplace_back(index, I);
		}

		signal_ReportProgress(static_cast<double>(sample) / numSeedSamples, false);
	}

	// --------------------------------------------------------------------------------

	// # Normalization factor & CDF

	// Normalization factor
	normFactor = sumI / Math::Float(numSeedSamples);

	// Create CDF
	for (auto& candidate : seedCandidates)
	{
		seedCandidateDist.Add(candidate.I);
	}

	seedCandidateDist.Normalize();

	// --------------------------------------------------------------------------------

	signal_ReportProgress(1, true);

	return true;
}

RenderProcess* PSSMLTRenderer::CreateRenderProcess(const Scene& scene, int threadID, int numThreads)
{
	if (seedCandidates.size() < static_cast<size_t>(numThreads))
	{
		LM_LOG_ERROR("Number of candidates is too small");
		return nullptr;
	}

	// Choose a seed
	const auto& seed = seedCandidates[seedCandidateDist.Sample(initialSampler->Next())];

	// Create a process
	std::unique_ptr<PSSMLTRenderer_RenderProcess> process(new PSSMLTRenderer_RenderProcess(*this, initialSampler->Clone(), pathSampler->Clone(), scene.MainCamera()->GetFilm()->Clone()));
	if (!process->Configure(scene, seed))
	{
		return nullptr;
	}

	return process.release();
}

// --------------------------------------------------------------------------------

bool PSSMLTRenderer_RenderProcess::Configure(const Scene& scene, const PSSMLTPathSeed& seed)
{
	// Configure and set seeds
	sampler->Configure(renderer.initialSampler->Rng()->Clone(), renderer.kernelSizeS1, renderer.kernelSizeS2);
	sampler->SetSeed(renderer.initialSampler->NextUInt());
	randomSampler->SetSeed(renderer.initialSampler->NextUInt());

	// Restore state of the seed path
	renderer.rewindableSampler->Rewind(seed.index);
	sampler->BeginRestore(*renderer.rewindableSampler);
	renderer.pathSampler->SampleAndEvaluate(scene, *sampler, Current(), renderer.rrDepth, -1);
	sampler->EndRestore();

	// Sanity check
	if (Math::Abs(Current().SumI() - seed.I) > Math::Constants::Eps())
	{
		LM_LOG_ERROR("Failed to reconstruct a seed path, invalid luminance");
		return false;
	}

	return true;
}

void PSSMLTRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	auto& current  = Current();
	auto& proposed = Proposed();

	// Enable large step mutation
	bool enableLargeStep = randomSampler->Next() < renderer.largeStepProb;
	sampler->EnableLargeStepMutation(enableLargeStep);

	// Sample and evaluate proposed path
	pathSampler->SampleAndEvaluate(scene, *sampler, proposed, renderer.rrDepth, -1);

	// Compute acceptance ratio
	auto currentI  = current.SumI();
	auto proposedI = proposed.SumI();
	auto a = Math::IsZero(currentI) ? Math::Float(1) : Math::Min(Math::Float(1), proposedI / currentI);

	// Determine accept or reject
	if (randomSampler->Next() < a)
	{
		sampler->Accept();
		currentIdx = 1 - currentIdx;
	}
	else
	{
		sampler->Reject();
	}

	// Accumulate contribution
	switch (renderer.estimatorMode)
	{
		case PSSMLTEstimatorMode::MeanValueSubstitution:
		{
			if (proposedI > Math::Float(0))
			{
				current.AccumulateContributionToFilm(*film, (1 - a) * renderer.normFactor / currentI);
				proposed.AccumulateContributionToFilm(*film, a * renderer.normFactor / proposedI);
			}
			else
			{
				current.AccumulateContributionToFilm(*film, renderer.normFactor / currentI);
			}
			break;
		}
		case PSSMLTEstimatorMode::MeanValueSubstitution_LargeStepMIS:
		{
			current.AccumulateContributionToFilm(*film, (1 - a) / (currentI / renderer.normFactor + renderer.largeStepProb));
			proposed.AccumulateContributionToFilm(*film, (a + (enableLargeStep ? Math::Float(1) : Math::Float(0))) / (proposedI / renderer.normFactor + renderer.largeStepProb));
			break;
		}
		case PSSMLTEstimatorMode::Normal:
		{
			auto& current = Current();
			current.AccumulateContributionToFilm(*film, renderer.normFactor / current.SumI());
			break;
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTRenderer, Renderer);

LM_NAMESPACE_END
