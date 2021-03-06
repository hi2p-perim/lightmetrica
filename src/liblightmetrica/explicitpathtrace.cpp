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
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

LM_NAMESPACE_BEGIN

enum class PathVertexType
{
	None,
	EndPoint,
	IntermediatePoint,
};

struct PathVertex
{

	// General information
	PathVertexType type;					// Vertex type
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

	PathVertex()
		: type(PathVertexType::None)
		, bsdf(nullptr)
		, emitter(nullptr)
	{

	}

};

// Object pool type for PathVertex.
typedef boost::object_pool<PathVertex, boost_pool_aligned_allocator<std::alignment_of<PathVertex>::value>> PathVertexPool;

struct Path
{

	Math::Vec2 rasterPos;
	std::vector<PathVertex*> vertices;

	void Add(PathVertex* vertex)
	{
		vertices.push_back(vertex);
	}

	void Release(PathVertexPool& pool)
	{
		for (auto* vertex : vertices)
		{
			pool.destroy(vertex);
		}
		vertices.clear();
	}

	Math::Vec2 RasterPosition() const { return rasterPos; }

};

/*
	Per-thread data.
	Contains data associated with a thread.
*/
struct ThreadContext
{
	
	std::unique_ptr<Random> rng;			// Random number generator
	std::unique_ptr<Film> film;				// Film
	std::unique_ptr<PathVertexPool> pool;	// Memory pool for path vertices

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

};

// --------------------------------------------------------------------------------

/*!
	Path tracing with explicit path sampling.
	This implementation of path tracing samples light paths
	and estimates LTE by explicitly evaluating the equation f / p.
*/
class ExplictPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("explicitpathtrace");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(0, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	bool SamplePath(const Scene& scene, Random& rng, PathVertexPool& pool, Path& path, Math::PDFEval& pathDimensionPdf);
	Math::Vec3 EvaluatePath(const Path& path, const Math::PDFEval& pathDimensionPdf);

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block
	std::string rngType;			// Type of random number generator

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;		// Experiments manager
#endif

};

bool ExplictPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
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
	if (!ComponentFactory::CheckRegistered<Random>(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

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

bool ExplictPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<ThreadContext> contexts;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		contexts.emplace_back(ComponentFactory::Create<Random>(rngType), masterFilm->Clone());
		contexts.back().rng->SetSeed(seed + i);
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

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

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
			film->AccumulateContribution(path.RasterPosition(), contrb);
			path.Release(*pool);

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
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	return true;
}

bool ExplictPathtraceRenderer::SamplePath( const Scene& scene, Random& rng, PathVertexPool& pool, Path& path, Math::PDFEval& pathDimensionPdf )
{
	PathVertex* v;

	// EyePosition
	v = pool.construct();
	v->type = PathVertexType::EndPoint;
	v->transportDir = TransportDirection::EL;
	v->emitter = scene.MainCamera();
	scene.MainCamera()->SamplePosition(rng.NextVec2(), v->geom, v->pdf);
	path.Add(v);

	// EyeDirection
	v = pool.construct();
	v->type = PathVertexType::IntermediatePoint;
	v->transportDir = TransportDirection::EL;
	v->bsdf = scene.MainCamera();
	v->geom = path.vertices[0]->geom;

	GeneralizedBSDFSampleQuery bsdfSQE;
	path.rasterPos = rng.NextVec2();
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

		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// Create path vertex
		v = pool.construct();

		// Surface geometry
		v->geom = isect.geom;

		const auto* light = isect.primitive->light;
		if (light)
		{
			// If the intersected vertex is light, decide
			// continuation of path sampling with probability 1/2
			pathDimensionPdf.v *= Math::Float(0.5);
			if (rng.Next() < Math::Float(0.5))
			{
				// Directional component
				v->type = PathVertexType::IntermediatePoint;
				v->transportDir = TransportDirection::LE;
				v->bsdf = light;
				v->wo = -ray.d;
				v->pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::ProjectedSolidAngle);	// TODO : Seems nasty
				path.Add(v);

				// Positional component
				v = pool.construct();
				v->type = PathVertexType::EndPoint;
				v->transportDir = TransportDirection::LE;
				v->emitter = light;
				v->geom = path.vertices.back()->geom;
				v->pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Area);	// TODO : Seems nasty
				path.Add(v);

				return true;
			}
		}

		// Otherwise vertex type is surface interaction
		v->type = PathVertexType::IntermediatePoint;
		v->transportDir = TransportDirection::EL;
		v->bsdf = isect.primitive->bsdf;
		v->wi = -ray.d;

		// --------------------------------------------------------------------------------

		// Sample BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.uComp = rng.Next();
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.wi = v->wi;
		
		GeneralizedBSDFSampleResult bsdfSR;
		if (!v->bsdf->SampleDirection(bsdfSQ, v->geom, bsdfSR))
		{
			pool.destroy(v);
			break;
		}

		v->wo = bsdfSR.wo;
		v->pdf = bsdfSR.pdf;

		// --------------------------------------------------------------------------------

		if (++depth >= rrDepth)
		{
			// Russian roulette for path termination
			// TODO : replace it
			Math::Float p(0.5);
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

Math::Vec3 ExplictPathtraceRenderer::EvaluatePath( const Path& path, const Math::PDFEval& pathDimensionPdf )
{
	Math::Vec3 contrb(Math::Float(1));

	for (const auto* v : path.vertices)
	{
		if (v->type == PathVertexType::EndPoint)
		{
			// Evaluate positional component of emitter
			contrb *= v->emitter->EvaluatePosition(v->geom) / v->pdf.v;
			LM_ASSERT(v->pdf.measure == Math::ProbabilityMeasure::Area);
		}
		else if (v->type == PathVertexType::IntermediatePoint)
		{
			// Evaluate generalized BSDF
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = v->transportDir;
			bsdfEQ.type = GeneralizedBSDFType::All;
			bsdfEQ.wi = v->wi;
			bsdfEQ.wo = v->wo;
			auto bsdf = v->bsdf->EvaluateDirection(bsdfEQ, v->geom);

			// Calculate contribution according to measure
			LM_ASSERT(v->pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
			contrb *= bsdf / v->pdf.v;
		}
	}

	return contrb / pathDimensionPdf.v;
}

LM_COMPONENT_REGISTER_IMPL(ExplictPathtraceRenderer, Renderer);

LM_NAMESPACE_END