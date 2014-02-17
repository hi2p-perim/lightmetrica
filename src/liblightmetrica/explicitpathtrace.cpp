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
#include <lightmetrica/explicitpathtrace.h>
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
#include <thread>
#include <atomic>
#include <omp.h>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

LM_NAMESPACE_BEGIN

/*
	Aligned allocator for boost::pool.
*/
template <std::size_t Align>
struct boost_pool_aligned_allocator
{

	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	static char* malloc(const size_type bytes) { return static_cast<char*>(aligned_malloc(bytes, Align)); }
	static void free(const char* block) { aligned_free((void*)block); }

};

/*
	Vertex type.
*/
enum PathVertexType
{
	// Primitive types
	EyePosition			= 1<<0,
	EyeDirection		= 1<<1,
	SurfaceInteraction	= 1<<2,
	LightDirection		= 1<<3,
	LightPosition		= 1<<4,

	// Useful flags
	EndPoint			= EyePosition | LightPosition,
	IntermediatePoint	= EyeDirection | SurfaceInteraction | LightDirection,
	
	// Surface interaction + emitter direction
	GeneralizedSurfaceInteraction = IntermediatePoint
};

/*
	Light path vertex.
*/
struct PathVertex
{
	
	// General information
	PathVertexType type;					// Vertex type
	SurfaceGeometry geom;					// Surface geometry information
	Math::PDFEval pdf;						// PDF evaluation

	// Generalized BSDF information
	TransportDirection transportDir;		// Transport direction
	const GeneralizedBSDF* bsdf;			// Generalized BSDF
	
	// Ray directions
	Math::Vec3 wi;							// Incoming ray
	Math::Vec3 wo;							// Outgoing ray in #dir

};

// Object pool type for PathVertex.
typedef boost::object_pool<PathVertex, boost_pool_aligned_allocator<std::alignment_of<PathVertex>::value>> PathVertexPool;

/*
	Light path.
*/
struct Path
{

	/*
	*/
	void Add(PathVertex* vertex)
	{
		vertices.push_back(vertex);
	}

	/*
	*/
	void Release(PathVertexPool& pool)
	{
		for (auto* vertex : vertices)
		{
			pool.destroy(vertex);
		}
		
		vertices.clear();
	}

	/*
	*/
	Math::Vec2 RasterPosition() const { return rasterPos; }

	Math::Vec2 rasterPos;
	std::vector<PathVertex*> vertices;

};

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct ThreadContext
{
	
	ThreadContext(Random* rng, Film* film)
		: rng(rng)
		, film(film)
		, pool(new PathVertexPool(sizeof(PathVertex)))
	{

	}

	ThreadContext (ThreadContext&& context)
		: rng(std::move(context.rng))
		, film(std::move(context.film))
		, pool(new PathVertexPool(sizeof(PathVertex)))
	{

	}

	std::unique_ptr<Random> rng;			// Random number generator
	std::unique_ptr<Film> film;				// Film
	std::unique_ptr<PathVertexPool> pool;	// Memory pool for path vertices

};

// --------------------------------------------------------------------------------

class ExplictPathtraceRenderer::Impl : public Object
{
public:

	Impl(ExplictPathtraceRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	bool SamplePath(const Scene& scene, Random& rng, PathVertexPool& pool, Path& path, Math::PDFEval& pathDimensionPdf);
	Math::Vec3 EvaluatePath(const Path& path, const Math::PDFEval& pathDimensionPdf);

private:

	ExplictPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block

};

ExplictPathtraceRenderer::Impl::Impl( ExplictPathtraceRenderer* self )
	: self(self)
{
	
}

bool ExplictPathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
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

	return true;
}

bool ExplictPathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<ThreadContext> contexts;
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

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			// Sample an eye sub-path
			Path path;
			Math::PDFEval pathDimensionPdf;
			if (!SamplePath(scene, *rng, *pool, path, pathDimensionPdf))
			{
				path.Release(*pool);
				continue;
			}

			// Evaluate contribution
			auto contrb = EvaluatePath(path, pathDimensionPdf);
			if (Math::IsZero(contrb))
			{
				path.Release(*pool);
				continue;
			}

			// Record to the film
			film->AccumulateContribution(path.RasterPosition(), contrb * Math::Float(film->Width() * film->Height()) / Math::Float(numSamples));
			path.Release(*pool);
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

bool ExplictPathtraceRenderer::Impl::SamplePath( const Scene& scene, Random& rng, PathVertexPool& pool, Path& path, Math::PDFEval& pathDimensionPdf )
{
	PathVertex* v;

	// EyePosition
	v = pool.construct();
	v->type = PathVertexType::EyePosition;
	v->bsdf = scene.MainCamera();
	path.rasterPos = rng.NextVec2(); 
	scene.MainCamera()->SamplePosition(rng.NextVec2(), v->geom, v->pdf);
	path.Add(v);

	// EyeDirection
	v = pool.construct();
	v->type = PathVertexType::EyeDirection;
	v->bsdf = scene.MainCamera();
	v->geom = path.vertices[0]->geom;
	GeneralizedBSDFSampleQuery bsdfSQE;
	bsdfSQE.sample = path.rasterPos;
	bsdfSQE.transportDir = TransportDirection::EL;
	bsdfSQE.type = GeneralizedBSDFType::EyeDirection;
	GeneralizedBSDFSampleResult bsdfSRE;
	scene.MainCamera()->SampleDirection(bsdfSQE, v->geom, bsdfSRE);
	v->pdf = bsdfSRE.pdf;
	v->wo = bsdfSRE.wo;
	path.Add(v);

	// --------------------------------------------------------------------------------

	int depth = 0;
	pathDimensionPdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);

	while (true)
	{
		// Create ray
		Ray ray;
		auto* pv = path.vertices.back();
		ray.d = pv->wo;
		ray.o = pv->geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Create path vertex
		v = pool.construct();
		v->wi = -ray.d;

		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			pool.destroy(v);
			break;
		}

		v->geom = isect.geom;

		const auto* light = v->isect.primitive->light;
		if (light)
		{
			// If the intersected vertex is light, decide
			// continuation of path sampling with probability 1/2
			pathDimensionPdf.v *= Math::Float(0.5);
			if (rng.Next() < Math::Float(0.5))
			{
				// Directional component
				v->type = PathVertexType::LightDirection;
				v->bsdf = light;
				path.Add(v);

				// Positional component
				v = pool.construct();
				v->type = PathVertexType::LightPosition;
				v->bsdf = light;
				path.Add(v);

				return true;
			}
		}

		// Otherwise vertex type is surface interaction
		v->type = PathVertexType::SurfaceInteraction;
		v->bsdf = isect.primitive->bsdf;

		// --------------------------------------------------------------------------------

		// Sample BSDF
		BSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.type = BSDFType::All;
		bsdfSQ.transportDir = TransportDirection::CameraToLight;
		bsdfSQ.wi = v->isect.worldToShading * v->wi;

		BSDFSampleResult bsdfSR;
		if (!v->isect.primitive->bsdf->Sample(bsdfSQ, bsdfSR) || bsdfSR.pdf.measure != Math::ProbabilityMeasure::SolidAngle)
		{
			pool.destroy(v);
			break;
		}

		v->wo = v->isect.shadingToWorld * bsdfSR.wo;
		v->pdf = bsdfSR.pdf;

		// --------------------------------------------------------------------------------

		if (++depth >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p(0.5); // = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (rng.Next() > p)
			{
				pool.destroy(v);
				break;
			}

			pathDimensionPdf.v *= p;
		}

		path.Add(v);
	}

	return false;
}

Math::Vec3 ExplictPathtraceRenderer::Impl::EvaluatePath( const Path& path, const Math::PDFEval& pathDimensionPdf )
{
	Math::Vec3 contrb(Math::Float(1));

	for (const auto* v : path.vertices)
	{
		if (v->type == PathVertexType::EyePosition)
		{
			// Evaluate positional component of We
			contrb *= v->camera->EvaluatePositionalWe(v->p) / v->pdf.v;
			LM_ASSERT(v->pdf.measure == Math::ProbabilityMeasure::Area);
		}
		else if (v->type == PathVertexType::LightPosition)
		{
			// Evaluate positional component of Le
			contrb *= v->light->EvaluatePositionalLe(v->p);
		}
		else if ((v->type & PathVertexType::IntermediatePoint) != 0)
		{
			if (v->type == PathVertexType::EyeDirection)
			{
				// Evaluate directional component of We
				contrb *= v->camera->EvaluateDirectionalWe(v->p, v->wo) / v->pdf.v;
				LM_ASSERT(v->pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
			else if (v->type == PathVertexType::LightDirection)
			{
				// Evaluate directional component of Le
				contrb *= v->light->EvaluateDirectionalLe(v->p, v->gn, v->wi);
			}
			else
			{
				// Evaluate BSDF
				BSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::CameraToLight;
				bsdfEQ.type = BSDFType::All;
				bsdfEQ.wi = v->isect.worldToShading * v->wi;
				bsdfEQ.wo = v->isect.worldToShading * v->wo;

				contrb *= v->isect.primitive->bsdf->Evaluate(bsdfEQ, v->isect) * Math::CosThetaZUp(bsdfEQ.wo) / v->pdf.v;
				LM_ASSERT(v->pdf.measure == Math::ProbabilityMeasure::SolidAngle);
			}
		}
		else
		{
			LM_LOG_ERROR("Invalid vertex type");
			return Math::Vec3();
		}
	}

	return contrb / pathDimensionPdf.v;
}

// --------------------------------------------------------------------------------

ExplictPathtraceRenderer::ExplictPathtraceRenderer()
	: p(new Impl(this))
{

}

ExplictPathtraceRenderer::~ExplictPathtraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool lightmetrica::ExplictPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool lightmetrica::ExplictPathtraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection lightmetrica::ExplictPathtraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END