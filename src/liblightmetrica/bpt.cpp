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
#include <thread>
#include <atomic>
#include <omp.h>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

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
*/
struct BPTPathVertex
{

	// General information
	BPTPathVertexType type;					// Vertex type
	SurfaceGeometry geom;					// Surface geometry information
	Math::PDFEval pdf;						// PDF evaluation

	// Generalized BSDF information
	TransportDirection transportDir;		// Transport direction
	const GeneralizedBSDF* bsdf;			// Generalized BSDF

	// For #type is EndPoint
	const Emitter* emitter;

	// Ray directions
	Math::Vec3 wi;							// Incoming ray
	Math::Vec3 wo;							// Outgoing ray in #dir

	BPTPathVertex()
		: type(BPTPathVertexType::None)
		, bsdf(nullptr)
		, emitter(nullptr)
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
		{
			LM_LOG_DEBUG("PDF");
			LM_LOG_INDENTER();
			LM_LOG_DEBUG("Measure : " +
				std::string(
					pdf.measure == Math::ProbabilityMeasure::Area ? "Area" :
					pdf.measure == Math::ProbabilityMeasure::SolidAngle ? "SolidAngle" :
					pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle ? "ProjectedSolidAngle" : "Discrete"));
			LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdf.v));
		}

		if (type == BPTPathVertexType::EndPoint)
		{
			LM_LOG_DEBUG("Emitter type : " + emitter->Name() + " (" + emitter->Type() + ")");
		}
		else if (type == BPTPathVertexType::IntermediatePoint)
		{
			LM_LOG_DEBUG("Generalized BSDF type : " + bsdf->Name() + " (" + bsdf->Type() + ")");
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

	void SampleSubpath(const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath);
	void EvaluateSubpathCombinations(const Scene& scene, Film& film, const BPTPath& eyeSubpath, const BPTPath& lightSubpath);

private:

	BidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block

	// Experimental parameters
	int maxSubpathLength;
	std::string subpathImageDir;

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

	// Experimental parameters
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		node.ChildValueOrDefault("max_subpath_length", 3, maxSubpathLength);
		node.ChildValueOrDefault<std::string>("subpath_image_dir", "bpt", subpathImageDir);
	}

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
		BPTPath eyeSubpath;
		BPTPath lightSubpath;

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Release and clear paths
			eyeSubpath.Release(*pool);
			eyeSubpath.Clear();
			lightSubpath.Release(*pool);
			lightSubpath.Clear();

			// Sample sub-paths
			SampleSubpath(scene, *rng, *pool, TransportDirection::EL, eyeSubpath);
			SampleSubpath(scene, *rng, *pool, TransportDirection::LE, lightSubpath);

			// Debug print
#if 0
			{
				LM_LOG_DEBUG("Sample #" + std::to_string(sample));
				LM_LOG_INDENTER();
				{
					LM_LOG_DEBUG("eye subpath");
					LM_LOG_INDENTER();
					eyeSubpath.DebugPrint();
				}
				{
					LM_LOG_DEBUG("light subpath");
					LM_LOG_INDENTER();
					lightSubpath.DebugPrint();
				}
			}
#endif

			// Evaluate combination of sub-paths
			EvaluateSubpathCombinations(scene, *film, eyeSubpath, lightSubpath);
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

void BidirectionalPathtraceRenderer::Impl::SampleSubpath( const Scene& scene, Random& rng, PathVertexPool& pool, TransportDirection transportDir, BPTPath& subpath )
{
	BPTPathVertex* v;

	// Positional component of emitter
	if (transportDir == TransportDirection::EL)
	{
		// EyePosition
		v = pool.construct();
		v->type = BPTPathVertexType::EndPoint;
		v->transportDir = transportDir;
		v->emitter = scene.MainCamera();
		v->emitter->SamplePosition(rng.NextVec2(), v->geom, v->pdf);
		subpath.Add(v);
	}
	else
	{
		// LightPosition
		v = pool.construct();
		v->type = BPTPathVertexType::EndPoint;
		v->transportDir = transportDir;
		auto lightSampleP = rng.NextVec2();
		Math::PDFEval lightSelectionPdf;
		v->emitter = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
		v->emitter->SamplePosition(lightSampleP, v->geom, v->pdf);
		v->pdf.v *= lightSelectionPdf.v;
		subpath.Add(v);
	}

	// --------------------------------------------------------------------------------

	// Directional component of emitter
	v = pool.construct();
	v->type = BPTPathVertexType::IntermediatePoint;
	v->transportDir = transportDir;
	v->bsdf = subpath.vertices.back()->emitter;
	v->geom = subpath.vertices.back()->geom;

	GeneralizedBSDFSampleQuery bsdfSQE;
	bsdfSQE.sample = rng.NextVec2();
	bsdfSQE.transportDir = transportDir;
	bsdfSQE.type = GeneralizedBSDFType::AllEmitter;

	GeneralizedBSDFSampleResult bsdfSRE;
	v->bsdf->SampleDirection(bsdfSQE, v->geom, bsdfSRE);
	v->pdf = bsdfSRE.pdf;
	v->wo = bsdfSRE.wo;

	subpath.Add(v);

	// --------------------------------------------------------------------------------

	int length = 0;
	while (true)
	{
		// Break if the length of subpath exceeds the maximum
		if (++length >= maxSubpathLength)
		{
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
		v->pdf = bsdfSR.pdf;

		subpath.Add(v);
	}
}

void BidirectionalPathtraceRenderer::Impl::EvaluateSubpathCombinations( const Scene& scene, Film& film, const BPTPath& eyeSubpath, const BPTPath& lightSubpath )
{
	// Although original estimator for Veach's BPT is
	//     F = \sum_{s \ge 0} \sum_{t \ge 0} w_{s,t}(\bar{x}_{s,t})\frac{f_j(\bar{x}_{s,t})}{p_{s,t}(\bar{x}_{s,t})}
	// 

	for (size_t s = 0; s < eyeSubpath.vertices.size(); s++)
	{
		for (size_t t = 0; t < lightSubpath.vertices.size(); t++)
		{
			
		}
	}
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