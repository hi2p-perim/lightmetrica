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
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
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
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

#define LM_ENABLE_BPT_EXPERIMENTAL

LM_NAMESPACE_BEGIN

/*
	BPT path vertex type.
	Vertex type of #PathVertex.
*/
enum class BPTPathVertexType
{
	None,									// Uninitialized
	EndPoint,								// Endpoint (emitter)
	IntermediatePoint,						// Intermediate point (generalized BSDF)
};

/*
	BPT path vertex.
	Represents a light path vertex.
	TODO : Use unrestricted unions in future implementation.
*/
struct BPTPathVertex
{

	// General information
	BPTPathVertexType type;					// Vertex type
	SurfaceGeometry geom;					// Surface geometry information

	/*
		Variables associated with Emitter.
		#type is EndPoint
	*/
	Math::PDFEval pdfP;						// PDF evaluation for positional component
	const Emitter* emitter;

	/*
		Variables associated with generalized BSDF.
		#type is either EndPoint or IntermediatePoint
	*/
	Math::PDFEval pdfD;						// PDF evaluation for directional component
	TransportDirection transportDir;		// Transport direction
	const GeneralizedBSDF* bsdf;			// Generalized BSDF
	const Light* areaLight;					// Light associated with surface
	const Camera* areaCamera;				// Camera associated with surface
	Math::Vec3 wi;							// Incoming ray
	Math::Vec3 wo;							// Outgoing ray in #dir

	// --------------------------------------------------------------------------------

	BPTPathVertex()
		: type(BPTPathVertexType::None)
		, emitter(nullptr)
		, bsdf(nullptr)
		, areaLight(nullptr)
		, areaCamera(nullptr)
	{

	}

	void DebugPrint()
	{
		LM_LOG_DEBUG("Type : " +
			std::string(
			type == BPTPathVertexType::EndPoint ? "EndPoint" :
			type == BPTPathVertexType::IntermediatePoint ? "IntermediatePoint" : "None"));

		if (type == BPTPathVertexType::None)
		{
			return;
		}

		LM_LOG_DEBUG("Transport direction : " +
			std::string(transportDir == TransportDirection::EL ? "EL" : "LE"));

		{
			LM_LOG_DEBUG("Surface geometry");
			LM_LOG_INDENTER();
			LM_LOG_DEBUG("Degenerated : " + std::string(geom.degenerated ? "True" : "False"));
			LM_LOG_DEBUG(boost::str(boost::format("Position : (%f, %f, %f)") % geom.p.x % geom.p.y % geom.p.z));
			if (!geom.degenerated)
			{
				LM_LOG_DEBUG(boost::str(boost::format("Geometry normal : (%f, %f, %f)") % geom.gn.x % geom.gn.y % geom.gn.z));
				LM_LOG_DEBUG(boost::str(boost::format("Shading normal : (%f, %f, %f)") % geom.sn.x % geom.sn.y % geom.sn.z));
			}
		}

		if (type == BPTPathVertexType::EndPoint)
		{
			LM_LOG_DEBUG("Emitter type : " + emitter->Name() + " (" + emitter->Type() + ")");
			{
				LM_LOG_DEBUG("PDF (positional component)");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " +
					std::string(
					pdfP.measure == Math::ProbabilityMeasure::Area ? "Area" :
					pdfP.measure == Math::ProbabilityMeasure::SolidAngle ? "SolidAngle" :
					pdfP.measure == Math::ProbabilityMeasure::ProjectedSolidAngle ? "ProjectedSolidAngle" : "Discrete"));
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfP.v));
			}
		}
		else if (type == BPTPathVertexType::IntermediatePoint)
		{
			LM_LOG_DEBUG("Generalized BSDF type : " + bsdf->Name() + " (" + bsdf->Type() + ")");
			{
				LM_LOG_DEBUG("PDF (directional component)");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " +
					std::string(
					pdfD.measure == Math::ProbabilityMeasure::Area ? "Area" :
					pdfD.measure == Math::ProbabilityMeasure::SolidAngle ? "SolidAngle" :
					pdfD.measure == Math::ProbabilityMeasure::ProjectedSolidAngle ? "ProjectedSolidAngle" : "Discrete"));
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfD.v));
			}
		}
	}

};

// Object pool type for PathVertex.
typedef boost::object_pool<BPTPathVertex, boost_pool_aligned_allocator<std::alignment_of<BPTPathVertex>::value>> PathVertexPool;

/*
	BPT path.
	Represents a light path.
*/
struct BPTPath
{

	std::vector<BPTPathVertex*> vertices;

	void Clear()
	{
		vertices.clear();
	}

	void Add(BPTPathVertex* vertex)
	{
		vertices.push_back(vertex);
	}

	void Release(PathVertexPool& pool)
	{
		for (auto* vertex : vertices)
			pool.destroy(vertex);
		vertices.clear();
	}

	void DebugPrint()
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			LM_LOG_DEBUG("Vertex #" + std::to_string(i));
			LM_LOG_INDENTER();
			vertices[i]->DebugPrint();
		}
	}

};

// --------------------------------------------------------------------------------

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

	BPTThreadContext (BPTThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, pool(new PathVertexPool(sizeof(BPTPathVertex)))
	{

	}

};

// --------------------------------------------------------------------------------

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
		\param pool Memoryh pool for path vertex.
		\param transportDir Transport direction.
		\param subpath Sampled subpath.
	*/
	void SampleSubpath(const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath);

	/*
		Evaluate contribution with combination of sub-paths.
		See [Veach 1997] for details.
		\param scene Scene.
		\param film Film.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
	*/
	void EvaluateSubpathCombinations(const Scene& scene, Film& film, const BPTPath& lightSubpath, const BPTPath& eyeSubpath);

	/*
		Evaluate MIS weight w_{s,t}.
		\param s Index of vertex in light sub-path.
		\param t Index of vertex in eye-subpath.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
		\return MIS weight.
	*/
	Math::Float EvaluateMISWeight(int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath);

	/*
		Evaluate unweight contribution C^*_{s,t}.
		\param s Index of vertex in light sub-path.
		\param t Index of vertex in eye-subpath.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
		\param Contribution.
	*/
	Math::Vec3 EvaluateUnweightContribution(const Scene& scene, int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath, Math::Vec2& rasterPosition);

	/*
		Evaluate alpha of sub-paths.
		The function is called from #EvaluateUnweightContribution.
		\param vs Number of vertices in sub-path (#s or #t).
		\param transportDir Transport direction.
		\param subpath Light sub-path or eye sub-path.
		\param rasterPosition Raster position.
	*/
	Math::Vec3 EvaluateSubpathAlpha(int vs, TransportDirection transportDir, const BPTPath& subpath, Math::Vec2& rasterPosition);

private:

	BidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;				// Number of samples
	int rrDepth;						// Depth of beginning RR
	int numThreads;						// Number of threads
	long long samplesPerBlock;			// Samples to be processed per block

#ifdef LM_ENABLE_BPT_EXPERIMENTAL
	// Experimental parameters
	bool enableExperimentalMode;		// Enables experimental mode if true
	int maxSubpathNumVertices;			// Maximum number of vertices of sub-paths
	std::string subpathImageDir;		// Output directory of sub-path images

	// Films for sub-path images
	std::vector<std::unique_ptr<Film>> subpathFilms;
	std::unique_ptr<Film> subpathTableFilm;
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

#ifdef LM_ENABLE_BPT_EXPERIMENTAL
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

	return true;
}

bool BidirectionalPathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<BPTThreadContext> contexts;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		contexts.emplace_back(new Random(seed + i), masterFilm->Clone());
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

#ifdef LM_ENABLE_BPT_EXPERIMENTAL
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

	// Table
	subpathTableFilm.reset(new HDRBitmapFilm);
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

#ifdef LM_ENABLE_BPT_EXPERIMENTAL
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
		LM_LOG_INFO("Saving sub-path images");
		LM_LOG_INDENTER();
		for (int s = 0; s <= maxSubpathNumVertices; s++)
		{
			for (int t = 0; t <= maxSubpathNumVertices; t++)
			{
				auto path = boost::filesystem::path(subpathImageDir) / boost::str(boost::format("L%02d_S%02d_T%02d.hdr") % (s+t) % s % t);
				{
					LM_LOG_INFO("Saving " + path.string());
					LM_LOG_INDENTER();
					if (!subpathFilms[s*maxSubpathNumVertices+t]->Save(path.string()))
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

void BidirectionalPathtraceRenderer::Impl::SampleSubpath( const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath )
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
	v->pdfD = bsdfSRE.pdf;
	v->wo = bsdfSRE.wo;

	subpath.Add(v);

	// --------------------------------------------------------------------------------

	int depth = 0;
	while (true)
	{
		// Break if the number of vertices of the sub-path exceeds the maximum
		if (++depth >= rrDepth)
		{
#ifdef LM_ENABLE_BPT_EXPERIMENTAL
			if (enableExperimentalMode && depth >= maxSubpathNumVertices)
			{
				break;
			}
#endif

			// TODO : Apply RR
			break;
		}

		// --------------------------------------------------------------------------------

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
		v->pdfD = bsdfSR.pdf;

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

	for (int n = 0; n <= nE + nL; n++)
	{
		// Process full-path with length n+1 (sub-path edges + connecting edge)
		Math::Float sumWeight = 0;
		const int minS = Math::Max(0, n-nE);
		const int maxS = nL;
		for (int s = minS; s <= maxS; s++)
		{
			const int t = n - s;

			// Evaluate weighting function w_{s,t}
			Math::Float w = EvaluateMISWeight(s, t, lightSubpath, eyeSubpath);
			sumWeight += w;

			// Evaluate unweight contribution C^*_{s,t}
			Math::Vec2 rasterPosition;
			auto Cstar = EvaluateUnweightContribution(scene, s, t, lightSubpath, eyeSubpath, rasterPosition);
			if (Math::IsZero(Cstar))
			{
				continue;
			}

#ifdef LM_ENABLE_BPT_EXPERIMENTAL
			// Accumulation contribution to sub-path films
			if (enableExperimentalMode)
			{
				#pragma omp critical
				{
					//LM_LOG_DEBUG("Raster position : " + std::to_string(rasterPosition.x) + " " + std::to_string(rasterPosition.y));
					//LM_LOG_DEBUG("Cstar : " + std::to_string(Cstar.x) + " " + std::to_string(Cstar.y) + " " + std::to_string(Cstar.z));
					subpathFilms[s*maxSubpathNumVertices+t]->AccumulateContribution(
						rasterPosition,
						Cstar * Math::Float(film.Width() * film.Height()) / Math::Float(numSamples));
				}
			}
#endif

			// Evaluate contribution C_{s,t}
			auto C = w * Cstar;

			// Record to the film
			film.AccumulateContribution(rasterPosition, C * Math::Float(film.Width() * film.Height()) / Math::Float(numSamples));
		}

		// Sum of MIS weight must be one
		LM_ASSERT(Math::Abs(sumWeight - Math::Float(1)) < Math::Constants::Eps());
	}
}

Math::Float BidirectionalPathtraceRenderer::Impl::EvaluateMISWeight( int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath )
{
	// Simplest weight!!
	// TODO : Replace with power heuristics
	const int nE = static_cast<int>(eyeSubpath.vertices.size());
	const int nL = static_cast<int>(lightSubpath.vertices.size());
	const int n = s + t;
	const int minS = Math::Max(0, n-nE);
	const int maxS = nL;
	return Math::Float(1) / Math::Cast<Math::Float>(maxS - minS + 1);
}

Math::Vec3 BidirectionalPathtraceRenderer::Impl::EvaluateUnweightContribution( const Scene& scene, int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath, Math::Vec2& rasterPosition )
{
	// Evaluate \alpha^L_s
	auto alphaL = EvaluateSubpathAlpha(s, TransportDirection::LE, lightSubpath, rasterPosition);
	if (Math::IsZero(alphaL))
	{
		return Math::Vec3();
	}

	// Evaluate \alpha^E_t
	auto alphaE = EvaluateSubpathAlpha(t, TransportDirection::EL, eyeSubpath, rasterPosition);
	if (Math::IsZero(alphaE))
	{
		return Math::Vec3();
	}
	
	// --------------------------------------------------------------------------------

	// Evaluate c_{s,t}
	Math::Vec3 cst;
	
	if (s == 0 && t > 0)
	{
		// z_{t-1} is area light
		auto* v = eyeSubpath.vertices[t-1];
		if (v->areaLight)
		{
			// Camera emitter cannot be an light
			LM_ASSERT(t >= 1);

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
	else if (s > 0 && t == 0)
	{
		// y_{s-1} is area camera
		auto* v = lightSubpath.vertices[s-1];
		if (v->areaCamera)
		{
			// Light emitter cannot be an camera
			LM_ASSERT(s >= 1);

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
	else if (s > 0 && t > 0)
	{
		auto* vL = lightSubpath.vertices[s-1];
		auto* vE = eyeSubpath.vertices[t-1];

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
		if (t == 1)
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

Math::Vec3 BidirectionalPathtraceRenderer::Impl::EvaluateSubpathAlpha( int vs, TransportDirection transportDir, const BPTPath& subpath, Math::Vec2& rasterPosition )
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

				// Update #alphaL
				// TODO : Should we arrange sampled measure?
				if (v->pdfD.measure == Math::ProbabilityMeasure::SolidAngle)
				{
					alpha *= fs * Math::Dot(v->geom.gn, v->wo) / v->pdfD.v;
				}
				else if (v->pdfD.measure == Math::ProbabilityMeasure::ProjectedSolidAngle)
				{
					alpha *= fs / v->pdfD.v;
				}
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