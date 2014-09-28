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
	Veach's bidirectional path trace renderer.
	An implementation of bidirectional path tracing (BPT) according to Veach's paper.
	Reference:
		E. Veach and L. Guibas, Bidirectional estimators for light transport,
		Procs. of the Fifth Eurographics Workshop on Rendering, pp.147-162, 1994.
*/
class BidirectionalPathtraceRenderer : public Renderer
{
private:

	friend class BidirectionalPathtraceRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("bpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene);
	virtual bool Preprocess(const Scene& scene);
	virtual bool Postprocess() const;
	virtual RenderProcess* CreateRenderProcess(const Scene& scene) const;
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

private:

	int rrDepth;											//!< Depth of beginning RR
	int maxPathVertices;									//!< Maximum number of light path vertices
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler
	std::unique_ptr<BPTMISWeight> misWeight;				//!< MIS weighting function

private:

#if LM_ENABLE_BPT_EXPERIMENTAL
	bool enableExperimentalMode;							//!< Enables experimental mode if true
	int maxSubpathNumVertices;								//!< Maximum number of vertices of sub-paths
	std::string subpathImageDir;							//!< Output directory of sub-path images
#endif

private:

#if LM_ENABLE_BPT_EXPERIMENTAL
	std::vector<std::unique_ptr<BitmapFilm>> subpathFilms;					// Films for sub-path images
	std::unordered_map<int, std::unique_ptr<BitmapFilm>> perLengthFilms;	// Per length images
#endif

private:

#if 0
#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif
#endif

};

// --------------------------------------------------------------------------------

/*!
	Render process for BidirectionalPathtraceRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class BidirectionalPathtraceRenderer_RenderProcess : public RenderProcess
{
public:

	BidirectionalPathtraceRenderer_RenderProcess(const BidirectionalPathtraceRenderer& renderer, Sampler* sampler, Film* film)
		: renderer(renderer)
		, sampler(sampler)
		, film(film)
		, subpathL(TransportDirection::LE)
		, subpathE(TransportDirection::EL)
	{}

private:

	LM_DISABLE_COPY_AND_MOVE(BidirectionalPathtraceRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

private:

	const BidirectionalPathtraceRenderer& renderer;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<Film> film;

private:

									// Sub-paths are reused in the same thread in order to
									// avoid unnecessary memory allocation
	BPTPathVertexPool pool;			//!< Memory pool for path vertices
	BPTSubpath subpathL;			//!< Light subpath
	BPTSubpath subpathE;			//!< Eye subpath

};

// --------------------------------------------------------------------------------

bool BidirectionalPathtraceRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
{
	// Load parameters
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("max_path_vertices", -1, maxPathVertices);

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

#if 0
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
#endif

	return true;
}

bool BidirectionalPathtraceRenderer::Preprocess(const Scene& scene)
{
	signal_ReportProgress(1, true);

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

	return true;
}

bool BidirectionalPathtraceRenderer::Postprocess() const
{
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

RenderProcess* BidirectionalPathtraceRenderer::CreateRenderProcess(const Scene& scene) const
{
	auto* sampler = initialSampler->Clone();
	sampler->SetSeed(initialSampler->NextUInt());
	return new BidirectionalPathtraceRenderer_RenderProcess(*this, sampler, scene.MainCamera()->GetFilm()->Clone());
}

// --------------------------------------------------------------------------------

void BidirectionalPathtraceRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	// Release and clear paths
	pool.Release();
	subpathL.Clear();
	subpathE.Clear();

	// Sample sub-paths
	subpathL.Sample(scene, *sampler, pool, renderer.rrDepth, renderer.maxPathVertices);
	subpathE.Sample(scene, *sampler, pool, renderer.rrDepth, renderer.maxPathVertices);

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

	// --------------------------------------------------------------------------------

	// Here we rewrote the order of summation of Veach's estimator (equation 10.3 in [Veach 1997])
	// in order to apply MIS intuitively

	const int nL = static_cast<int>(subpathL.vertices.size());
	const int nE = static_cast<int>(subpathE.vertices.size());

	// For each subpath vertex sums n
	// If n = 0 or 1 no valid path is generated
	for (int n = 2; n <= nE + nL; n++)
	{
		if (renderer.maxPathVertices != -1 && n > renderer.maxPathVertices)
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
			BPTFullPath fullPath(s, t, subpathL, subpathE);

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
			auto w = renderer.misWeight->Evaluate(fullPath);

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulation contribution to sub-path films
			if (renderer.enableExperimentalMode && s <= renderer.maxSubpathNumVertices && t <= renderer.maxSubpathNumVertices)
			{
				#pragma omp critical
				{
					renderer.subpathFilms[s*(renderer.maxSubpathNumVertices+1)+t]->AccumulateContribution(rasterPosition, Cstar);
				}
			}
#endif

			// Evaluate contribution C_{s,t} and record to the film
			film->AccumulateContribution(rasterPosition, w * Cstar);

#if LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulate contribution to per length image
			if (renderer.enableExperimentalMode)
			{
				#pragma omp critical
				{
					// Extend if needed
					if (renderer.perLengthFilms.find(n) == renderer.perLengthFilms.end())
					{
						// Create a new film for #n vertices
						std::unique_ptr<BitmapFilm> newFilm(dynamic_cast<BitmapFilm*>(ComponentFactory::Create<Film>("hdr")));
						newFilm->SetImageType(BitmapImageType::RadianceHDR);
						newFilm->Allocate(film->Width(), film->Height());
						renderer.perLengthFilms.insert(std::make_pair(n, std::move(newFilm)));
					}

					// Accumulate
					renderer.perLengthFilms[n]->AccumulateContribution(rasterPosition, C);
				}
			}
#endif
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(BidirectionalPathtraceRenderer, Renderer);

LM_NAMESPACE_END