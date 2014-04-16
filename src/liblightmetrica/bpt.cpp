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
#include <lightmetrica/bpt.h>
#include <lightmetrica/bpt.path.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/hdrfilm.h>
#include <lightmetrica/random.h>
#include <lightmetrica/randomfactory.h>
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
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

#define LM_ENABLE_BPT_EXPERIMENTAL 1

LM_NAMESPACE_BEGIN

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct BPTThreadContext
{
	
	std::unique_ptr<Random> rng;			// Random number generator
	std::unique_ptr<Film> film;				// Film
	std::unique_ptr<PathVertexPool> pool;	// Memory pool for path vertices

	BPTThreadContext(Random* rng, Film* film)
		: rng(rng)
		, film(film)
		, pool(new PathVertexPool(sizeof(BPTPathVertex)))
	{

	}

	BPTThreadContext(BPTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, pool(new PathVertexPool(sizeof(BPTPathVertex)))
	{

	}

};

// --------------------------------------------------------------------------------

enum class BPTMISWeightMode
{
	Simple,				// Reciprocal of # of non-zero probability sampling strategies
	PowerHeuristics		// Power heuristics
};

class BidirectionalPathtraceRenderer::Impl : public Object
{
public:

	Impl(BidirectionalPathtraceRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	/*
		Sample a sub-path.
		Sample eye sub-subpath or light sub-path according to #transportDir.
		\param scene Scene.
		\param rng Random number generator.
		\param pool Memory pool for path vertex.
		\param transportDir Transport direction.
		\param subpath Sampled subpath.
	*/
	void SampleSubpath(const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath) const;

	/*
		Evaluate contribution with combination of sub-paths.
		See [Veach 1997] for details.
		\param scene Scene.
		\param film Film.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
	*/
	void EvaluateSubpathCombinations(const Scene& scene, Film& film, const BPTPath& lightSubpath, const BPTPath& eyeSubpath);

private:

	/*
		Evaluate MIS weight w_{s,t}.
		\param fullPath Full-path.
		\return MIS weight.
	*/
	Math::Float EvaluateMISWeight(const BPTFullPath& fullPath) const;

	// MIS weight functions
	Math::Float EvaluateSimpleMISWeight(const BPTFullPath& fullPath) const;
	Math::Float EvaluatePowerHeuristicsMISWeight(const BPTFullPath& fullPath) const;

	// Evaluate p_{i+1}(\bar{x}_{s,t})/p_i(\bar{x}_{s,t}).
	Math::Float EvaluateSubsequentProbRatio(int i, const BPTFullPath& fullPath) const;

	// Get i-th vertex of the full-path
	const BPTPathVertex* FullPathVertex(int i, const BPTFullPath& fullPath) const;

	// Get i-th directional PDF evaluation of the full-path
	Math::PDFEval FullPathVertexDirectionPDF(int i, const BPTFullPath& fullPath, TransportDirection transportDir) const;

private:

	/*
		Evaluate unweight contribution C^*_{s,t}.
		\param s Index of vertex in light sub-path.
		\param t Index of vertex in eye-subpath.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
		\param Contribution.
	*/
	Math::Vec3 EvaluateUnweightContribution(const Scene& scene, const BPTFullPath& fullPath, Math::Vec2& rasterPosition) const;

	/*
		Evaluate alpha of sub-paths.
		The function is called from #EvaluateUnweightContribution.
		\param vs Number of vertices in sub-path (#s or #t).
		\param transportDir Transport direction.
		\param subpath Light sub-path or eye sub-path.
		\param rasterPosition Raster position.
	*/
	Math::Vec3 EvaluateSubpathAlpha(int vs, TransportDirection transportDir, const BPTPath& subpath, Math::Vec2& rasterPosition) const;

private:

	BidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;						// Number of samples
	int rrDepth;								// Depth of beginning RR
	int numThreads;								// Number of threads
	long long samplesPerBlock;					// Samples to be processed per block
	std::string rngType;						// Type of random number generator
	
	BPTMISWeightMode misWeightMode;				// MIS weight function
	Math::Float misPowerHeuristicsBetaCoeff;	// Beta coefficient for power heuristics

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Experimental parameters
	bool enableExperimentalMode;				// Enables experimental mode if true
	int maxSubpathNumVertices;					// Maximum number of vertices of sub-paths
	std::string subpathImageDir;				// Output directory of sub-path images

	// Films for sub-path images
	std::vector<std::unique_ptr<Film>> subpathFilms;
	// Per length images
	std::unordered_map<int, std::unique_ptr<Film>> perLengthFilms;
#endif

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

BidirectionalPathtraceRenderer::Impl::Impl( BidirectionalPathtraceRenderer* self )
	: self(self)
{

}

bool BidirectionalPathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
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
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!RandomFactory::CheckSupport(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

	// MIS weight function
	auto misWeightModeNode = node.Child("mis_weight");
	if (misWeightModeNode.Empty())
	{
		misWeightMode = BPTMISWeightMode::PowerHeuristics;
		misPowerHeuristicsBetaCoeff = Math::Float(2);
		LM_LOG_WARN("Missing 'mis_weight' element. Using default value.");
	}
	else
	{
		auto modeNode = misWeightModeNode.Child("mode");
		if (modeNode.Empty())
		{
			misWeightMode = BPTMISWeightMode::PowerHeuristics;
			misPowerHeuristicsBetaCoeff = Math::Float(2);
			LM_LOG_WARN("Missing 'mis_weight' element. Using default value.");
		}
		else
		{
			if (modeNode.Value() == "simple")
			{
				misWeightMode = BPTMISWeightMode::Simple;
			}
			else if (modeNode.Value() == "power_heuristics")
			{
				misWeightMode = BPTMISWeightMode::PowerHeuristics;
				misWeightModeNode.ChildValueOrDefault("beta_coeff", Math::Float(2), misPowerHeuristicsBetaCoeff);
			}
			else
			{
				LM_LOG_ERROR("Invalid MIS weight mode '" + modeNode.Value() + "'");
				return false;
			}
		}
	}

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Experimental parameters
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		enableExperimentalMode = true;
		experimentalNode.ChildValueOrDefault("max_subpath_num_vertices", 3, maxSubpathNumVertices);
		experimentalNode.ChildValueOrDefault<std::string>("subpath_image_dir", "bpt", subpathImageDir);
	}
	else
	{
		enableExperimentalMode = false;
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

bool BidirectionalPathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<BPTThreadContext> contexts;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		contexts.emplace_back(RandomFactory::Create(rngType), masterFilm->Clone());
		contexts.back().rng->SetSeed(seed + i);
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
				subpathFilms.emplace_back(masterFilm->Clone());
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
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		// Sub-paths are reused in the sample block
		// but probably it should be shared by per thread configuration
		// (TODO : check performance gain)
		BPTPath lightSubpath;
		BPTPath eyeSubpath;

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Release and clear paths
			lightSubpath.Release(*pool);
			lightSubpath.Clear();
			eyeSubpath.Release(*pool);
			eyeSubpath.Clear();

			// Sample sub-paths
			SampleSubpath(scene, *rng, *pool, TransportDirection::LE, lightSubpath);
			SampleSubpath(scene, *rng, *pool, TransportDirection::EL, eyeSubpath);

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
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& context : contexts)
	{
		masterFilm->AccumulateContribution(*context.film.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

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

void BidirectionalPathtraceRenderer::Impl::SampleSubpath( const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath ) const
{
	BPTPathVertex* v;

	// Initial vertex
	v = pool.construct();
	v->type = BPTPathVertexType::EndPoint;
	v->transportDir = transportDir;

	// Positional component
	if (transportDir == TransportDirection::EL)
	{
		// EyePosition
		v->emitter = scene.MainCamera();
		v->emitter->SamplePosition(rng.NextVec2(), v->geom, v->pdfP);
	}
	else
	{
		// LightPosition
		auto lightSampleP = rng.NextVec2();
		Math::PDFEval lightSelectionPdf;
		v->emitter = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
		v->emitter->SamplePosition(lightSampleP, v->geom, v->pdfP);
		v->pdfP.v *= lightSelectionPdf.v;
	}

	// Directional component
	v->bsdf = v->emitter;

	GeneralizedBSDFSampleQuery bsdfSQE;
	bsdfSQE.sample = rng.NextVec2();
	bsdfSQE.transportDir = transportDir;
	bsdfSQE.type = GeneralizedBSDFType::AllEmitter;

	GeneralizedBSDFSampleResult bsdfSRE;
	v->bsdf->SampleDirection(bsdfSQE, v->geom, bsdfSRE);
	v->pdfD[transportDir] = bsdfSRE.pdf;
	v->pdfD[1-transportDir] = Math::PDFEval();
	v->wo = bsdfSRE.wo;

	// # of vertices is always greater than 1
	v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);

	subpath.Add(v);

	// --------------------------------------------------------------------------------

	int depth = 1;
	while (true)
	{
		// Previous vertex
		auto* pv = subpath.vertices.back();

		// Create ray
		Ray ray;
		ray.d = pv->wo;
		ray.o = pv->geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// --------------------------------------------------------------------------------

		// Create path vertex
		v = pool.construct();
		v->type = BPTPathVertexType::IntermediatePoint;
		v->transportDir = transportDir;
		v->bsdf = isect.primitive->bsdf;
		v->geom = isect.geom;
		v->wi = -pv->wo;

		// Area light or camera
		v->areaLight = isect.primitive->light;
		v->areaCamera = isect.primitive->camera;

		// Area light or camera should not be associated with the same surfaces
		LM_ASSERT(v->areaLight == nullptr || v->areaCamera == nullptr);
		if (v->areaLight)
		{
			v->emitter = v->areaLight;
		}
		if (v->areaCamera)
		{
			v->emitter = v->areaCamera;
		}
		if (v->emitter)
		{
			// Calculate #pdfP for intersected emitter
			v->pdfP = v->emitter->EvaluatePositionPDF(v->geom);
		}
		else
		{
			v->pdfP = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::Area);
		}

		// --------------------------------------------------------------------------------

		// Apply RR
		int rrDepthT = rrDepth;
#if LM_ENABLE_BPT_EXPERIMENTAL
		if (enableExperimentalMode)
		{
			// At least #maxSubpathNumVertices vertices are sampled in the experimental mode
			rrDepthT = Math::Max(rrDepthT, maxSubpathNumVertices);
		}
#endif

		if (++depth >= rrDepthT)
		{
			// TODO : Replace with the more efficient one
			auto p = Math::Float(0.5);
			if (rng.Next() > p)
			{
				subpath.Add(v);
				break;
			}

			// RR probability
			v->pdfRR = Math::PDFEval(p, Math::ProbabilityMeasure::Discrete);
		}
		else
		{
			v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.transportDir = transportDir;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = -pv->wo;

		GeneralizedBSDFSampleResult bsdfSR;
		if (!v->bsdf->SampleDirection(bsdfSQ, v->geom, bsdfSR))
		{
			subpath.Add(v);
			break;
		}

		v->wo = bsdfSR.wo;
		v->pdfD[transportDir] = bsdfSR.pdf;

		// Evaluate PDF in the opposite transport direction
		// TODO : Handle specular case
		GeneralizedBSDFEvaluateQuery bsdfEQ;
		bsdfEQ.type = bsdfSR.sampledType;
		bsdfEQ.transportDir = TransportDirection(1 - transportDir);
		bsdfEQ.wi = bsdfSR.wo;
		bsdfEQ.wo = bsdfSQ.wi;
		v->pdfD[1-transportDir] = v->bsdf->EvaluateDirectionPDF(bsdfEQ, v->geom);

		subpath.Add(v);
	}
}

void BidirectionalPathtraceRenderer::Impl::EvaluateSubpathCombinations( const Scene& scene, Film& film, const BPTPath& lightSubpath, const BPTPath& eyeSubpath )
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
			const int t = n - s;

			// Create full-path
			BPTFullPath fullPath(s, t, lightSubpath, eyeSubpath);

			// Evaluate weighting function w_{s,t}
			Math::Float w = EvaluateMISWeight(fullPath);
			sumWeight += w;

			// Evaluate unweight contribution C^*_{s,t}
			Math::Vec2 rasterPosition;
			auto Cstar = EvaluateUnweightContribution(scene, fullPath, rasterPosition);
			if (Math::IsZero(Cstar))
			{
				continue;
			}

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

			// Evaluate contribution C_{s,t}
			auto C = w * Cstar;

			// Record to the film
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
						std::unique_ptr<HDRBitmapFilm> newFilm(new HDRBitmapFilm(""));
						newFilm->SetHDRImageType(HDRImageType::RadianceHDR);
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

Math::Float BidirectionalPathtraceRenderer::Impl::EvaluateMISWeight( const BPTFullPath& fullPath ) const
{
	if (misWeightMode == BPTMISWeightMode::Simple)
	{
		return EvaluateSimpleMISWeight(fullPath);
	}
	else
	{
		LM_ASSERT(misWeightMode == BPTMISWeightMode::PowerHeuristics);
		return EvaluatePowerHeuristicsMISWeight(fullPath);
	}
}

Math::Float BidirectionalPathtraceRenderer::Impl::EvaluateSimpleMISWeight( const BPTFullPath& fullPath ) const
{
	// Simple weight: reciprocal of # of full-paths with positive probability.
	const int nE = static_cast<int>(fullPath.eyeSubpath.vertices.size());
	const int nL = static_cast<int>(fullPath.lightSubpath.vertices.size());
	const int n = fullPath.s + fullPath.t;

	int minS = Math::Max(0, n-nE);
	int maxS = Math::Min(nL, n);

	// This is tricky part, in the weight calculation
	// we need to exclude samples with zero probability.
	// We note that if the last vertex of sub-path with s=0 or t=0
	// is not on non-pinhole camera or area light, the path probability is zero,
	// because we cannot sample degenerated points.
	int nonZeroProbPaths = n+1;
	if (fullPath.lightSubpath.vertices[0]->geom.degenerated)
	{
		nonZeroProbPaths--;
		if (fullPath.s == 0)
		{
			return Math::Float(0);
		}
	}
	if (fullPath.eyeSubpath.vertices[0]->geom.degenerated)
	{
		nonZeroProbPaths--;
		if (fullPath.t == 0)
		{
			return Math::Float(0);
		}
	}

	return Math::Float(1) / Math::Float(nonZeroProbPaths);
}

Math::Float BidirectionalPathtraceRenderer::Impl::EvaluatePowerHeuristicsMISWeight( const BPTFullPath& fullPath ) const
{
	const int n = fullPath.s + fullPath.t;

	// Inverse of the weight 1/w_{s,t}. Initial weight is p_s/p_s = 1
	Math::Float invWeight(1);
	Math::Float piDivPs;

	// Iteratively compute p_i/p_s where i = s-1 downto 0
	piDivPs = Math::Float(1);
	for (int i = fullPath.s-1; i >= 0; i--)
	{
		Math::Float ratio = EvaluateSubsequentProbRatio(i, fullPath);
		if (Math::IsZero(ratio))
		{
			break;
		}
		piDivPs *= ratio;
		invWeight += piDivPs * piDivPs;
	}

	// Iteratively compute p_i/p_s where i = s+1 to n
	piDivPs = Math::Float(1);
	for (int i = fullPath.s+1; i < n; i++)
	{
		Math::Float ratio = EvaluateSubsequentProbRatio(i, fullPath);
		if (Math::IsZero(ratio))
		{
			break;
		}
		piDivPs *= Math::Float(1) / ratio;
		invWeight += piDivPs * piDivPs;
	}

	LM_LOG_DEBUG(std::to_string(invWeight));

	return Math::Float(1) / invWeight;
}

Math::Float BidirectionalPathtraceRenderer::Impl::EvaluateSubsequentProbRatio( int i, const BPTFullPath& fullPath ) const
{
	int n = fullPath.s + fullPath.t;
	if (i == 0)
	{
		// p_1 / p_0 =
		//    p_A(x_0) /
		//    p_{\sigma^\bot}(x_1\to x_0)G(x_1\leftrightarrow x_0)
		const auto* x0	= FullPathVertex(i, fullPath);
		const auto* x1	= FullPathVertex(i+1, fullPath);
		auto x1PdfDEL	= FullPathVertexDirectionPDF(i+1, fullPath, TransportDirection::EL);

		if (Math::IsZero(x0->pdfP.v))
		{
			return Math::Float(0);
		}
		else
		{
			LM_ASSERT(x0->pdfP.measure == Math::ProbabilityMeasure::Area);
			LM_ASSERT(x1PdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

			return
				x0->pdfP.v / x1PdfDEL.v /
				RenderUtils::GeneralizedGeometryTerm(x0->geom, x1->geom);
		}
	}
	else if (i == n-1)
	{
		// p_n / p_{n-1} =
		//     p_{\sigma^\bot}(x_{n-2}\to x_{n-1})G(x_{n-2}\leftrightarrow x_{n-1}) /
		//     p_A(x_{n-1})
		const auto* xn		= FullPathVertex(n-1, fullPath);
		const auto* xnPrev	= FullPathVertex(n-2, fullPath);
		auto xnPrevPdfDLE	= FullPathVertexDirectionPDF(n-2, fullPath, TransportDirection::LE);

		if (Math::IsZero(xn->pdfP.v))
		{
			return Math::Float(0);
		}
		else
		{
			LM_ASSERT(xn->pdfP.measure == Math::ProbabilityMeasure::Area);
			LM_ASSERT(xnPrevPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

			return
				xnPrevPdfDLE.v *
				RenderUtils::GeneralizedGeometryTerm(xnPrev->geom, xn->geom) /
				xn->pdfP.v;
		}
	}
	else
	{
		// p_{i+1} / p_i =
		//     p_{\sigma^\bot}(x_{i-1}\to x_i)G(x_{i-1}\leftrightarrow x_i) /
		//     p_{\sigma^\bot}(x_{i+1}\to x_i)G(x_{i+1}\leftrightarrow x_i)
		const auto* xi		= FullPathVertex(i, fullPath);
		const auto* xiNext	= FullPathVertex(i+1, fullPath);
		const auto* xiPrev	= FullPathVertex(i-1, fullPath);
		auto xiPrevPdfDLE	= FullPathVertexDirectionPDF(i-1, fullPath, TransportDirection::LE);
		auto xiNextPdfDEL	= FullPathVertexDirectionPDF(i+1, fullPath, TransportDirection::EL);

		LM_ASSERT(xiPrevPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
		LM_ASSERT(xiNextPdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

		return
			xiPrevPdfDLE.v *
			RenderUtils::GeneralizedGeometryTerm(xiPrev->geom, xi->geom) /
			xiNextPdfDEL.v /
			RenderUtils::GeneralizedGeometryTerm(xiNext->geom, xi->geom);
	}

	return Math::Float(0);
}

const BPTPathVertex* BidirectionalPathtraceRenderer::Impl::FullPathVertex( int i, const BPTFullPath& fullPath ) const
{
	LM_ASSERT(0 <= i && i < fullPath.s + fullPath.t);
	return
		i < fullPath.s
			? fullPath.lightSubpath.vertices[i]
			: fullPath.eyeSubpath.vertices[fullPath.t-1-(i-fullPath.s)];
}

Math::PDFEval BidirectionalPathtraceRenderer::Impl::FullPathVertexDirectionPDF( int i, const BPTFullPath& fullPath, TransportDirection transportDir ) const
{
	LM_ASSERT(0 <= i && i < fullPath.s + fullPath.t);
	return
		i == fullPath.s - 1	? fullPath.pdfDL[transportDir] :
		i == fullPath.s		? fullPath.pdfDE[transportDir]
							: FullPathVertex(i, fullPath)->pdfD[transportDir];
}

Math::Vec3 BidirectionalPathtraceRenderer::Impl::EvaluateUnweightContribution( const Scene& scene, const BPTFullPath& fullPath, Math::Vec2& rasterPosition ) const
{
	// Evaluate \alpha^L_s
	auto alphaL = EvaluateSubpathAlpha(fullPath.s, TransportDirection::LE, fullPath.lightSubpath, rasterPosition);
	if (Math::IsZero(alphaL))
	{
		return Math::Vec3();
	}

	// Evaluate \alpha^E_t
	auto alphaE = EvaluateSubpathAlpha(fullPath.t, TransportDirection::EL, fullPath.eyeSubpath, rasterPosition);
	if (Math::IsZero(alphaE))
	{
		return Math::Vec3();
	}
	
	// --------------------------------------------------------------------------------

	// Evaluate c_{s,t}
	Math::Vec3 cst;
	
	if (fullPath.s == 0 && fullPath.t > 0)
	{
		// z_{t-1} is area light
		auto* v = fullPath.eyeSubpath.vertices[fullPath.t-1];
		if (v->areaLight)
		{
			// Camera emitter cannot be an light
			LM_ASSERT(fullPath.t >= 1);

			// Evaluate Le^0(z_{t-1})
			cst = v->areaLight->EvaluatePosition(v->geom);

			// Evaluate Le^1(z_{t-1}\to z_{t-2})
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wo = v->wi;
			cst *= v->areaLight->EvaluateDirection(bsdfEQ, v->geom);
		}
	}
	else if (fullPath.s > 0 && fullPath.t == 0)
	{
		// y_{s-1} is area camera
		auto* v = fullPath.lightSubpath.vertices[fullPath.s-1];
		if (v->areaCamera)
		{
			// Light emitter cannot be an camera
			LM_ASSERT(fullPath.s >= 1);

			// Raster position
			if (v->areaCamera->RayToRasterPosition(v->geom.p, v->wi, rasterPosition))
			{
				// Evaluate We^0(y_{s-1})
				cst = v->areaCamera->EvaluatePosition(v->geom);

				// Evaluate We^1(y_{s-1}\to y_{s-2})
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.wo = v->wi;
				cst *= v->areaCamera->EvaluateDirection(bsdfEQ, v->geom);
			}
		}
	}
	else if (fullPath.s > 0 && fullPath.t > 0)
	{
		auto* vL = fullPath.lightSubpath.vertices[fullPath.s-1];
		auto* vE = fullPath.eyeSubpath.vertices[fullPath.t-1];

		// Check connectivity between #vL->geom.p and #vE->geom.p
		Ray shadowRay;
		auto pLpE = vE->geom.p - vL->geom.p;
		auto pLpE_Length = Math::Length(pLpE);
		shadowRay.d = pLpE / pLpE_Length;
		shadowRay.o = vL->geom.p;
		shadowRay.minT = Math::Constants::Eps();
		shadowRay.maxT = pLpE_Length * (Math::Float(1) - Math::Constants::Eps());

		// Update raster position if #t = 1
		bool visible = true;
		if (fullPath.t == 1)
		{
			visible = scene.MainCamera()->RayToRasterPosition(vE->geom.p, -shadowRay.d, rasterPosition);
		}

		Intersection shadowIsect;
		if (visible && !scene.Intersect(shadowRay, shadowIsect))
		{			
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::All;

			// fsL
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = vL->wi;
			bsdfEQ.wo = shadowRay.d;
			auto fsL = vL->bsdf->EvaluateDirection(bsdfEQ, vL->geom);

			// fsE
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = vE->wi;
			bsdfEQ.wo = -shadowRay.d;
			auto fsE = vE->bsdf->EvaluateDirection(bsdfEQ, vE->geom);

			// Geometry term
			auto G = RenderUtils::GeneralizedGeometryTerm(vL->geom, vE->geom);

			cst = fsL * G * fsE;
		}
	}

	if (Math::IsZero(cst))
	{
		return Math::Vec3();
	}

	// --------------------------------------------------------------------------------

	// Evaluate contribution C^*_{s,t} = \alpha^L_s * c_{s,t} * \alpha^E_t
	return alphaL * cst * alphaE;
}

Math::Vec3 BidirectionalPathtraceRenderer::Impl::EvaluateSubpathAlpha( int vs, TransportDirection transportDir, const BPTPath& subpath, Math::Vec2& rasterPosition ) const
{
	Math::Vec3 alpha;

	if (vs == 0)
	{
		// \alpha_0 = 1
		alpha = Math::Vec3(Math::Float(1));
	}
	else
	{
		BPTPathVertex* v = subpath.vertices[0];

		LM_ASSERT(v->type == BPTPathVertexType::EndPoint);
		LM_ASSERT(v->emitter != nullptr);
		LM_ASSERT(v->pdfP.measure == Math::ProbabilityMeasure::Area);

		// Calculate raster position if transport direction is EL
		bool visible = true;
		if (transportDir == TransportDirection::EL)
		{
			visible = dynamic_cast<const Camera*>(v->emitter)->RayToRasterPosition(v->geom.p, v->wo, rasterPosition);
		}
		
		if (visible)
		{
			// Emitter
			// \alpha^L_1 = Le^0(y0) / p_A(y0) or \alpha^E_1 = We^0(z0) / p_A(z0)
			alpha = v->emitter->EvaluatePosition(v->geom) / v->pdfP.v;

			for (int i = 0; i < vs - 1; i++)
			{
				v = subpath.vertices[i];

				// f_s(y_{i-1}\to y_i\to y_{i+1}) or f_s(z_{i-1}\to z_i\to z_{i+1})
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = GeneralizedBSDFType::All;
				bsdfEQ.transportDir = transportDir;
				bsdfEQ.wi = v->wi;
				bsdfEQ.wo = v->wo;
				auto fs = v->bsdf->EvaluateDirection(bsdfEQ, v->geom);

				// Update #alphaL or #alphaE
				LM_ASSERT(v->pdfD[transportDir].measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				alpha *= fs / v->pdfD[transportDir].v;

				// RR probability
				LM_ASSERT(v->pdfRR.measure == Math::ProbabilityMeasure::Discrete);
				alpha /= v->pdfRR.v;
			}
		}
	}

	return alpha;
}

// --------------------------------------------------------------------------------

BidirectionalPathtraceRenderer::BidirectionalPathtraceRenderer()
	: p(new Impl(this))
{

}

BidirectionalPathtraceRenderer::~BidirectionalPathtraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool BidirectionalPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool BidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection BidirectionalPathtraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END