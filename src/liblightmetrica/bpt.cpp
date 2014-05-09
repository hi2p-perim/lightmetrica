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
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.mis.h>
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
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Per-thread data.
	Contains data associated with a thread.
*/
struct BPTThreadContext
{
	
	std::unique_ptr<Random> rng;				//!< Random number generator
	std::unique_ptr<Film> film;					//!< Film
	std::unique_ptr<BPTPathVertexPool> pool;	//!< Memory pool for path vertices

	BPTThreadContext(Random* rng, Film* film)
		: rng(rng)
		, film(film)
		, pool(new BPTPathVertexPool)
	{

	}

	BPTThreadContext(BPTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, pool(new BPTPathVertexPool)
	{

	}

};

/*!
	Veach's bidirectional path trace renderer.
	An implementation of bidirectional path tracing (BPT) according to Veach's paper.
	Reference:
		Veach, E. and Guibas, L., Bidirectional estimators for light transport,
		In Proceedings of the Fifth Eurographics Workshop on Rendering, pp. 147-162, 1994.
*/
class BidirectionalPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("bpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	/*!
		Evaluate contribution with combination of sub-paths.
		See [Veach 1997] for details.
		\param scene Scene.
		\param film Film.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
	*/
	void EvaluateSubpathCombinations(const Scene& scene, Film& film, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath);

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;
	BPTConfig config;

#if LM_ENABLE_BPT_EXPERIMENTAL
	std::vector<std::unique_ptr<BitmapFilm>> subpathFilms;					// Films for sub-path images
	std::unordered_map<int, std::unique_ptr<BitmapFilm>> perLengthFilms;	// Per length images
#endif

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool BidirectionalPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	if (!config.Load(node, assets))
	{
		return false;
	}

#if LM_EXPERIMENTAL_MODE
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

		if (config.numThreads != 1)
		{
			LM_LOG_WARN("Number of thread must be 1 in experimental mode, forced 'num_threads' to 1");
			config.numThreads = 1;
		}
	}
#endif

	return true;
}

bool BidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(config.numThreads);

	// Random number generators and films
	std::vector<BPTThreadContext> contexts;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < config.numThreads; i++)
	{
		contexts.emplace_back(ComponentFactory::Create<Random>(config.rngType), masterFilm->Clone());
		contexts.back().rng->SetSeed(seed + i);
	}

	// Number of blocks to be separated
	long long blocks = (config.numSamples + config.samplesPerBlock) / config.samplesPerBlock;

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Initialize films for sub-path combinations
	if (config.enableExperimentalMode)
	{
		for (int s = 0; s <= config.maxSubpathNumVertices; s++)
		{
			for (int t = 0; t <= config.maxSubpathNumVertices; t++)
			{
				std::unique_ptr<BitmapFilm> film(dynamic_cast<BitmapFilm*>(ComponentFactory::Create<Film>("hdr")));
				film->SetImageType(BitmapImageType::RadianceHDR);
				film->Allocate(masterFilm->Width(), masterFilm->Height());
				subpathFilms.push_back(std::move(film));
			}
		}
	}
#endif

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
		long long sampleBegin = config.samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + config.samplesPerBlock, config.numSamples);

		// Sub-paths are reused in the sample block
		// but probably it should be shared by per thread configuration
		// (TODO : check performance gain)
		BPTSubpath lightSubpath(TransportDirection::LE);
		BPTSubpath eyeSubpath(TransportDirection::EL);

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Release and clear paths
			lightSubpath.Release(*pool);
			eyeSubpath.Release(*pool);

			// Sample sub-paths
			lightSubpath.Sample(config, scene, *rng, *pool);
			eyeSubpath.Sample(config, scene, *rng, *pool);

			// Debug print
#if 0
			{
				LM_LOG_DEBUG("Sample #" + std::to_string(sample));
				{
					LM_LOG_DEBUG("light subpath");
					LM_LOG_INDENTER();
					lightSubpath.DebugPrint();
				}
				LM_LOG_INDENTER();
				{
					LM_LOG_DEBUG("eye subpath");
					LM_LOG_INDENTER();
					eyeSubpath.DebugPrint();
				}
			}
#endif

			// Evaluate combination of sub-paths
			EvaluateSubpathCombinations(scene, *film, lightSubpath, eyeSubpath);

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
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context.film.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(config.numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

#if LM_ENABLE_BPT_EXPERIMENTAL
	if (config.enableExperimentalMode)
	{
		// Create output directory if it does not exists
		if (!boost::filesystem::exists(config.subpathImageDir))
		{
			LM_LOG_INFO("Creating directory " + config.subpathImageDir);
			if (!boost::filesystem::create_directory(config.subpathImageDir))
			{
				return false;
			}
		}

		// Save sub-path images
		{
			LM_LOG_INFO("Saving sub-path images");
			LM_LOG_INDENTER();
			for (int s = 0; s <= config.maxSubpathNumVertices; s++)
			{
				for (int t = 0; t <= config.maxSubpathNumVertices; t++)
				{
					auto path = boost::filesystem::path(config.subpathImageDir) / boost::str(boost::format("s%02dt%02d.hdr") % s % t);
					{
						LM_LOG_INFO("Saving " + path.string());
						LM_LOG_INDENTER();
						auto& film = subpathFilms[s*(config.maxSubpathNumVertices+1)+t];
						if (!film->RescaleAndSave(path.string(), Math::Float(film->Width() * film->Height()) / Math::Float(config.numSamples)))
						{
							return false;
						}
					}
				}
			}
		}

		// Save per length images
		{
			LM_LOG_INFO("Saving per length images");
			LM_LOG_INDENTER();
			for (const auto& kv : perLengthFilms)
			{
				auto path = boost::filesystem::path(config.subpathImageDir) / boost::str(boost::format("l%03d.hdr") % kv.first);
				{
					LM_LOG_INFO("Saving " + path.string());
					LM_LOG_INDENTER();
					auto& film = kv.second;
					if (!film->RescaleAndSave(path.string(), Math::Float(film->Width() * film->Height()) / Math::Float(config.numSamples)))
					{
						return false;
					}
				}
			}
		}
	}
#endif

	return true;
}

void BidirectionalPathtraceRenderer::EvaluateSubpathCombinations( const Scene& scene, Film& film, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath )
{
	// Let n_E and n_L be the number of vertices of eye and light sub-paths respectively.
	// Rewriting the order of summation of Veach's estimator (equation 10.3 in [Veach 1997]),
	// the estimate for the j-th pixel contribution is written as
	//     I_j = \sum_{n=0}^{n_E+n_L} \sum_{s=0}^l w_{s,n-s}(\bar{x}_{s,n-s})\frac{f_j(\bar{x}_{s,n-s})}{p_{s,l-s}(\bar{x}_{s,n-s})},
	// where
	//     - \bar{x}_{s,n-s} : Path sampled from p_{s,n-s}
	//     - w_{s,n-s} : MIS weight

	const int nL = static_cast<int>(lightSubpath.vertices.size());
	const int nE = static_cast<int>(eyeSubpath.vertices.size());

	// If n = 0 or 1 no valid path is generated
	for (int n = 2; n <= nE + nL; n++)
	{
		// Process full-path with length n+1 (sub-path edges + connecting edge)
		Math::Float sumWeight = 0;
		const int minS = Math::Max(0, n-nE);
		const int maxS = Math::Min(nL, n);
		for (int s = minS; s <= maxS; s++)
		{
			// Number of vertices in eye sub-path
			const int t = n - s;

			// Create full-path
			BPTFullPath fullPath(s, t, lightSubpath, eyeSubpath);

			// Evaluate weighting function w_{s,t}
			Math::Float w = config.misWeight->Evaluate(fullPath);
			sumWeight += w;

			// Evaluate unweight contribution C^*_{s,t}
			Math::Vec2 rasterPosition;
			auto Cstar = fullPath.EvaluateUnweightContribution(scene, rasterPosition);
			if (Math::IsZero(Cstar))
			{
				continue;
			}

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulation contribution to sub-path films
			if (config.enableExperimentalMode && s <= config.maxSubpathNumVertices && t <= config.maxSubpathNumVertices)
			{
				#pragma omp critical
				{
					subpathFilms[s*(config.maxSubpathNumVertices+1)+t]->AccumulateContribution(rasterPosition, Cstar);
				}
			}
#endif

			// Evaluate contribution C_{s,t}
			auto C = w * Cstar;

			// Record to the film
			film.AccumulateContribution(rasterPosition, C);

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulate contribution to per length image
			if (config.enableExperimentalMode)
			{
				#pragma omp critical
				{
					// Extend if needed
					if (perLengthFilms.find(n) == perLengthFilms.end())
					{
						// Create a new film for #n vertices
						std::unique_ptr<BitmapFilm> newFilm(dynamic_cast<BitmapFilm*>(ComponentFactory::Create<Film>("hdr")));
						newFilm->SetImageType(BitmapImageType::RadianceHDR);
						newFilm->Allocate(film.Width(), film.Height());
						perLengthFilms.insert(std::make_pair(n, std::move(newFilm)));
					}

					// Accumulate
					perLengthFilms[n]->AccumulateContribution(rasterPosition, C);
				}
			}
#endif
		}

		// Sum of MIS weight must be one.
		// Check only if all sampling strategies is available except the case terminating by RR.
		if (minS == 0 && maxS == n)
		{
			LM_ASSERT(Math::Abs(sumWeight - Math::Float(1)) < Math::Constants::Eps());
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(BidirectionalPathtraceRenderer, Renderer);

LM_NAMESPACE_END