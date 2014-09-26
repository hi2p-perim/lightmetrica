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
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/configurablesampler.h>
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
#include <lightmetrica/defaultexperiments.h>
#include <lightmetrica/bitmapfilm.h>
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
	
	std::unique_ptr<Sampler> sampler;			//!< Sampler
	std::unique_ptr<Film> film;					//!< Film
	std::unique_ptr<BPTPathVertexPool> pool;	//!< Memory pool for path vertices

	// Sub-paths are reused in the same thread in order to
	// avoid unnecessary memory allocation
	BPTSubpath lightSubpath;					//!< Light subpath
	BPTSubpath eyeSubpath;						//!< Eye subpath

	BPTThreadContext(Sampler* sampler, Film* film)
		: sampler(sampler)
		, film(film)
		, pool(new BPTPathVertexPool)
		, lightSubpath(TransportDirection::LE)
		, eyeSubpath(TransportDirection::EL)
	{

	}

	BPTThreadContext(BPTThreadContext&& context)
		: sampler(std::move(context.sampler))
		, film(std::move(context.film))
		, pool(new BPTPathVertexPool)
		, lightSubpath(TransportDirection::LE)
		, eyeSubpath(TransportDirection::EL)
	{

	}

};

/*!
	Veach's bidirectional path trace renderer.
	An implementation of bidirectional path tracing (BPT) according to Veach's paper.
	Reference:
		E. Veach and L. Guibas, Bidirectional estimators for light transport,
		Procs. of the Fifth Eurographics Workshop on Rendering, pp.147-162, 1994.
*/
class BidirectionalPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("bpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene);
	virtual void SetTerminationMode( TerminationMode mode, double time ) { terminationMode = mode; terminationTime = time; }
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(1, true); return true; }
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
	TerminationMode terminationMode;
	double terminationTime;

	long long numSamples;									//!< Number of samples
	int rrDepth;											//!< Depth of beginning RR
	int maxPathVertices;									//!< Maximum number of light path vertices
	int numThreads;											//!< Number of threads
	long long samplesPerBlock;								//!< Samples to be processed per block
	Math::Float intermediateDuration;						//!< Seconds between intermediate images are output (if -1, disabled)
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler
	std::unique_ptr<BPTMISWeight> misWeight;				//!< MIS weighting function

#if LM_ENABLE_BPT_EXPERIMENTAL
	bool enableExperimentalMode;		//!< Enables experimental mode if true
	int maxSubpathNumVertices;			//!< Maximum number of vertices of sub-paths
	std::string subpathImageDir;		//!< Output directory of sub-path images
#endif

#if LM_ENABLE_BPT_EXPERIMENTAL
	std::vector<std::unique_ptr<BitmapFilm>> subpathFilms;					// Films for sub-path images
	std::unordered_map<int, std::unique_ptr<BitmapFilm>> perLengthFilms;	// Per length images
#endif

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool BidirectionalPathtraceRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
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
	node.ChildValueOrDefault("intermediate_duration", Math::Float(-1), intermediateDuration);

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

	// MIS weight function
	auto misWeightModeNode = node.Child("mis_weight");
	if (misWeightModeNode.Empty())
	{
		LM_LOG_ERROR("Missing 'mis_weight' element");
		return false;
	}
	auto misWeightType = misWeightModeNode.AttributeValue("type");
	if (!ComponentFactory::CheckRegistered<BPTMISWeight>(misWeightType))
	{
		LM_LOG_ERROR("Unsupported MIS weighting function '" + misWeightType + "'");
		return false;
	}
	auto* p = ComponentFactory::Create<BPTMISWeight>(misWeightType);
	if (p == nullptr)
	{
		return false;
	}
	misWeight.reset(p);
	if (!p->Configure(misWeightModeNode, assets))
	{
		return false;
	}

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Experimental parameters
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		enableExperimentalMode = true;
		experimentalNode.ChildValueOrDefault("max_subpath_num_vertices", 3, maxSubpathNumVertices);
		experimentalNode.ChildValueOrDefault<std::string>("subpath_image_dir", "bpt", subpathImageDir);

		// At least #maxSubpathNumVertices vertices are sampled in the experimental mode
		rrDepth = Math::Max(rrDepth, maxSubpathNumVertices);
	}
	else
	{
		enableExperimentalMode = false;
	}
#endif

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

		if (numThreads != 1)
		{
			LM_LOG_WARN("Number of thread must be 1 in experimental mode, forced 'num_threads' to 1");
			numThreads = 1;
		}
	}
#endif

	return true;
}

bool BidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);
	std::atomic<long long> processedSamples(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<BPTThreadContext> contexts;
	for (int i = 0; i < numThreads; i++)
	{
		contexts.emplace_back(initialSampler->Clone(), masterFilm->Clone());
		contexts.back().sampler->SetSeed(initialSampler->NextUInt());
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Initialize films for sub-path combinations
	if (enableExperimentalMode)
	{
		for (int s = 0; s <= maxSubpathNumVertices; s++)
		{
			for (int t = 0; t <= maxSubpathNumVertices; t++)
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

	bool cancel = false;
	bool done = false;
	auto startTime = std::chrono::high_resolution_clock::now();
	auto prevStartTime = startTime;
	int intermediateImageOutputCount = 0;

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

			// Thread ID
			int threadId = omp_get_thread_num();
			auto& sampler = contexts[threadId].sampler;
			auto& film    = contexts[threadId].film;
			auto& pool    = contexts[threadId].pool;
			auto& lightSubpath = contexts[threadId].lightSubpath;
			auto& eyeSubpath   = contexts[threadId].eyeSubpath;

			// Sample range
			long long sampleBegin = samplesPerBlock * block;
			long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

			processedSamples += sampleEnd - sampleBegin;
			LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

			for (long long sample = sampleBegin; sample < sampleEnd; sample++)
			{
				// Release and clear paths
				pool->Release();
				lightSubpath.Clear();
				eyeSubpath.Clear();

				// Sample sub-paths
				lightSubpath.Sample(scene, *sampler, *pool, rrDepth, maxPathVertices);
				eyeSubpath.Sample(scene, *sampler, *pool, rrDepth, maxPathVertices);

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

			//processedBlocks++;
			//auto progress = static_cast<double>(processedBlocks) / blocks;
			//signal_ReportProgress(progress, processedBlocks == blocks);

			// Progress report
			processedBlocks++;
			if (terminationMode == TerminationMode::Samples)
			{
				auto progress = static_cast<double>(processedBlocks) / blocks;
				signal_ReportProgress(progress, false);
				LM_EXPT_UPDATE_PARAM(expts, "progress", &progress);
			}
			else if (terminationMode == TerminationMode::Time)
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

			LM_EXPT_UPDATE_PARAM(expts, "block", &block);
			//LM_EXPT_UPDATE_PARAM(expts, "progress", &progress);
			LM_EXPT_NOTIFY(expts, "ProgressUpdated");
		}

		if (intermediateDuration > Math::Float(0))
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevStartTime).count()) / 1000.0;
			if (elapsed > static_cast<double>(intermediateDuration))
			{
				// Create output directory if it does not exists
				const std::string outputDir = "images";
				if (!boost::filesystem::exists(outputDir))
				{
					LM_LOG_INFO("Creating directory : " + outputDir);
					if (!boost::filesystem::create_directory(outputDir))
					{
						LM_LOG_WARN("Failed to create output directory : " + outputDir);
					}
				}

				// Same intermediate image
				masterFilm->Clear();
				for (auto& context : contexts)
				{
					masterFilm->AccumulateContribution(*context.film.get());
				}

				// Rescale & save
				intermediateImageOutputCount++;
				auto path = boost::filesystem::path(outputDir) / boost::str(boost::format("%010d.png") % intermediateImageOutputCount);
				dynamic_cast<BitmapFilm*>(masterFilm)->RescaleAndSave(
					path.string(), Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));

				LM_LOG_INFO("Saving : " + path.string());
				prevStartTime = currentTime;
			}
		}

		if (done || terminationMode == TerminationMode::Samples)
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
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context.film.get());
	}

	// Rescale master film
	//masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(processedSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

#if LM_ENABLE_BPT_EXPERIMENTAL
	if (enableExperimentalMode)
	{
		// Create output directory if it does not exists
		if (!boost::filesystem::exists(subpathImageDir))
		{
			LM_LOG_INFO("Creating directory " + subpathImageDir);
			if (!boost::filesystem::create_directory(subpathImageDir))
			{
				return false;
			}
		}

		// Save sub-path images
		{
			LM_LOG_INFO("Saving sub-path images");
			LM_LOG_INDENTER();
			for (int s = 0; s <= maxSubpathNumVertices; s++)
			{
				for (int t = 0; t <= maxSubpathNumVertices; t++)
				{
					auto path = boost::filesystem::path(subpathImageDir) / boost::str(boost::format("s%02dt%02d.hdr") % s % t);
					{
						LM_LOG_INFO("Saving " + path.string());
						LM_LOG_INDENTER();
						auto& film = subpathFilms[s*(maxSubpathNumVertices+1)+t];
						if (!film->RescaleAndSave(path.string(), Math::Float(film->Width() * film->Height()) / Math::Float(numSamples)))
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
				auto path = boost::filesystem::path(subpathImageDir) / boost::str(boost::format("l%03d.hdr") % kv.first);
				{
					LM_LOG_INFO("Saving " + path.string());
					LM_LOG_INDENTER();
					auto& film = kv.second;
					if (!film->RescaleAndSave(path.string(), Math::Float(film->Width() * film->Height()) / Math::Float(numSamples)))
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
	// Here we rewrote the order of summation of Veach's estimator (equation 10.3 in [Veach 1997])
	// in order to apply MIS intuitively

	const int nL = static_cast<int>(lightSubpath.vertices.size());
	const int nE = static_cast<int>(eyeSubpath.vertices.size());

	// For each subpath vertex sums n
	// If n = 0 or 1 no valid path is generated
	for (int n = 2; n <= nE + nL; n++)
	{
		if (maxPathVertices != -1 && n > maxPathVertices)
		{
			continue;
		}

		// Process full-path with length n+1 (subpath edges + connecting edge)
		const int minS = Math::Max(0, n-nE);
		const int maxS = Math::Min(nL, n);

		for (int s = minS; s <= maxS; s++)
		{
			// Number of vertices in eye subpath
			const int t = n - s;

			// Create fullpath
			BPTFullPath fullPath(s, t, lightSubpath, eyeSubpath);

			// Evaluate unweighted contribution C^*_{s,t}
			Math::Vec2 rasterPosition;
			auto Cstar = fullPath.EvaluateUnweightContribution(scene, rasterPosition);
			if (Math::IsZero(Cstar))
			{
				// For some reason, the sampled path is not used
				// e.g., y_{s-1} and z_{t-1} is not visible each other
				continue;
			}

			// Evaluate weighting function w_{s,t}
			auto w = misWeight->Evaluate(fullPath);

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulation contribution to sub-path films
			if (enableExperimentalMode && s <= maxSubpathNumVertices && t <= maxSubpathNumVertices)
			{
				#pragma omp critical
				{
					subpathFilms[s*(maxSubpathNumVertices+1)+t]->AccumulateContribution(rasterPosition, Cstar);
				}
			}
#endif

			// Evaluate contribution C_{s,t} and record to the film
			auto C = w * Cstar;
			film.AccumulateContribution(rasterPosition, C);

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulate contribution to per length image
			if (enableExperimentalMode)
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
	}
}

LM_COMPONENT_REGISTER_IMPL(BidirectionalPathtraceRenderer, Renderer);

LM_NAMESPACE_END