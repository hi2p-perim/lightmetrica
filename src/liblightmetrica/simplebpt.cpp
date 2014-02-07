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
#include <lightmetrica/simplebpt.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/random.h>
#include <lightmetrica/light.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/primitive.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

enum Subpath
{
	E = 0,		//!< Eye sub-path.
	L = 1		//!< Light sub-path.
};

struct GeneralizedBSDFSampleQuery
{

	Math::Vec2 sample;				//!< Uniform random numbers.

	int subpath;					//!< Which sub-path to be evaluated?
	int pathLength;					//!< Path length of specified sub-path.
	
	Math::Vec3 p;					//!< Current position.
	Math::Vec3 gn;					//!< Geometry normal at #p.
	Math::Vec3 wi;					//!< Input direction.

	const Camera* camera;			//!< Camera.
	const Light* light;				//!< Light.
	Intersection isect;				//!< Current intersection structure for specified sub-path.

};

struct GeneralizedBSDFSampleResult
{

	Math::Vec3 wo;					//!< Sampled outgoing direction.
	Math::PDFEval pdf;				//!< Evaluated PDF.

};

struct GeneralizedBSDFEvaluateQuery
{

	GeneralizedBSDFEvaluateQuery() {}
	GeneralizedBSDFEvaluateQuery(const GeneralizedBSDFSampleQuery& query, const GeneralizedBSDFSampleResult& result)
		: subpath(query.subpath)
		, pathLength(query.pathLength)
		, p(query.p)
		, gn(query.gn)
		, wi(query.wi)
		, wo(result.wo)
		, camera(query.camera)
		, light(query.light)
		, isect(query.isect)
	{

	}

	int subpath;					//!< Which sub-path to be evaluated?
	int pathLength;					//!< Path length of specified sub-path.

	Math::Vec3 p;					//!< Current position.
	Math::Vec3 gn;					//!< Geometry normal at #p.
	Math::Vec3 wi;					//!< Input direction.
	Math::Vec3 wo;					//!< Outgoing direction.

	const Camera* camera;			//!< Camera.
	const Light* light;				//!< Light.
	Intersection isect;				//!< Current intersection structure for specified sub-path.

};

class SimpleBidirectionalPathtraceRenderer::Impl : public Object
{
public:

	Impl(SimpleBidirectionalPathtraceRenderer* self);

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	bool Render( const Scene& scene );
	boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	/*
		Evaluate generalized BSDF.
		Ordinary BSDF + directional component of We and Le,
		which reduces troublesome case analysis.
	*/
	Math::Vec3 EvaluateGeneralizedBSDF(const GeneralizedBSDFEvaluateQuery& query) const;

	/*
		Sample generalized BSDF.
		Sampling from ordinary BSDF or We or Le according to path length.
	*/
	bool SampleGeneralizedBSDF(const GeneralizedBSDFSampleQuery& query, GeneralizedBSDFSampleResult& result) const;

	/*
		Evaluate generalized geometry term.
		Ordinary geometry term + degeneration support.
	*/
	Math::Float EvaluateGeneralizedGeometryTerm(Math::Vec3 currP[2], Math::Vec3 currGN[2], int pathLength[2]) const;

private:

	SimpleBidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block

};

SimpleBidirectionalPathtraceRenderer::Impl::Impl( SimpleBidirectionalPathtraceRenderer* self )
	: self(self)
{

}

bool SimpleBidirectionalPathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	// Check type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid renderer type '" + node.AttributeValue("type") + "'");
		return false;
	}

	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 0, rrDepth);
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

bool SimpleBidirectionalPathtraceRenderer::Impl::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Random>> rngs;
	std::vector<std::unique_ptr<Film>> films;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		rngs.push_back(std::unique_ptr<Random>(new Random(seed + i)));
		films.push_back(std::unique_ptr<Film>(masterFilm->Clone()));
	}

	// Number of blocks to be separated
	long long blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (long long block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = rngs[threadId];
		auto& film = films[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			Math::Vec3 pE;
			Math::Vec3 gnE;
			Math::PDFEval pdfPE;

			// Sample position on the camera
			scene.MainCamera()->SamplePosition(rng->NextVec2(), pE, gnE, pdfPE);
			
			// Evaluate We^{(0)} (positional component of We)
			auto positionalWe = scene.MainCamera()->EvaluatePositionalWe(pE);

			// --------------------------------------------------------------------------------

			Math::Vec3 pL;
			Math::Vec3 gnL;
			Math::PDFEval pdfPL;

			// Sample a position on the light
			auto lightSampleP = rng->NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, pL, gnL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Evaluate Le^{(0)} (positional component of Le)
			auto positionalLe = light->EvaluatePositionalLe(pL);

			// --------------------------------------------------------------------------------

			// Length of eye and light sub-paths respectively
			int pathLength[2] = { 0 };

			// Path throughputs
			Math::Vec3 throughput[2] = { positionalWe / pdfPE.v, positionalLe / pdfPL.v };

			// Current intersection structures
			Intersection currIsect[2];

			// Current and previous positions and geometry normals
			Math::Vec3 currP[2] = { pE, pL };
			Math::Vec3 currGN[2] = { gnE, gnL };
			Math::Vec3 currWi[2];

			// Raster position
			Math::Vec2 rasterPos;

			while (true)
			{
				// Check connectivity between #pE and #pL
				Ray shadowRay;
				auto pEpL = currP[Subpath::L] - currP[Subpath::E];
				auto pEpL_Length = Math::Length(pEpL);
				shadowRay.o = currP[Subpath::E];
				shadowRay.d = pEpL / pEpL_Length;
				shadowRay.minT = Math::Constants::Eps();
				shadowRay.maxT = pEpL_Length * (Math::Float(1) - Math::Constants::Eps());

				Intersection shadowIsect;
				if (!scene.Intersect(shadowRay, shadowIsect))
				{
					bool visible = true;
					if (pathLength[Subpath::E] == 0)
					{
						// If the length of eye sub-path is zero, compute raster position here
						visible = scene.MainCamera()->RayToRasterPosition(shadowRay.o, shadowRay.d, rasterPos);
					}

					// If visible in the screen ..
					if (visible)
					{
						// Generalized BSDFs
						GeneralizedBSDFEvaluateQuery bsdfEQ;
						bsdfEQ.camera = scene.MainCamera();
						bsdfEQ.light = light;

						// fsE
						bsdfEQ.subpath = Subpath::E;
						bsdfEQ.pathLength = pathLength[Subpath::E];
						bsdfEQ.p = currP[Subpath::E];
						bsdfEQ.gn = currGN[Subpath::E];
						bsdfEQ.wi = currWi[Subpath::E];
						bsdfEQ.wo = shadowRay.d;
						bsdfEQ.isect = currIsect[Subpath::E];
						auto fsE = EvaluateGeneralizedBSDF(bsdfEQ);

						// fsL
						bsdfEQ.subpath = Subpath::L;
						bsdfEQ.pathLength = pathLength[Subpath::L];
						bsdfEQ.p = currP[Subpath::L];
						bsdfEQ.gn = currGN[Subpath::L];
						bsdfEQ.wi = currWi[Subpath::L];
						bsdfEQ.wo = -shadowRay.d;
						bsdfEQ.isect = currIsect[Subpath::L];
						auto fsL = EvaluateGeneralizedBSDF(bsdfEQ);

						// Geometry term
						auto G = EvaluateGeneralizedGeometryTerm(currP, currGN, pathLength);

						// Evaluate contribution and accumulate to film
						auto contrb = throughput[Subpath::E] * fsE * G * fsL * throughput[Subpath::L];
						film->AccumulateContribution(rasterPos, contrb * Math::Float(film->Width() * film->Height()) / Math::Float(numSamples));
					}
				}

				// --------------------------------------------------------------------------------

				// Select which sub-path to be extended
				// This selection might be tricky. TODO : Add some explanation
				int subpath = rng->Next() < Math::Float(0.5) ? Subpath::E : Subpath::L;

				// Decide if the selected -path is actually extended by Russian roulette
				// If the sub-path is not extended, terminate the loop.
				if (pathLength[subpath] >= rrDepth)
				{
					auto p = Math::Min(Math::Float(0.5), Math::Luminance(throughput[subpath]));
					if (rng->Next() > p)
					{
						break;
					}
					else
					{
						throughput[subpath] /= p;
					}
				}

				// --------------------------------------------------------------------------------

				// Sample generalized BSDF
				GeneralizedBSDFSampleQuery bsdfSQ;
				bsdfSQ.sample = rng->NextVec2();
				bsdfSQ.subpath = subpath;
				bsdfSQ.pathLength = pathLength[subpath];
				bsdfSQ.p = currP[subpath];
				bsdfSQ.gn = currGN[subpath];
				bsdfSQ.wi = currWi[subpath];
				bsdfSQ.camera = scene.MainCamera();
				bsdfSQ.light = light;
				bsdfSQ.isect = currIsect[subpath];

				GeneralizedBSDFSampleResult bsdfSR;
				if (!SampleGeneralizedBSDF(bsdfSQ, bsdfSR))
				{
					break;
				}
					
				// Evaluate generalized BSDF
				auto fs = EvaluateGeneralizedBSDF(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR));
				if (Math::IsZero(fs))
				{
					break;
				}

				// Update throughput
				// Evaluation of generalized BSDF may evaluate its PDF in two measures (for degeneration support)
				if (bsdfSR.pdf.measure == Math::ProbabilityMeasure::SolidAngle)
				{
					throughput[subpath] *= fs * Math::Dot(currGN[subpath], bsdfSR.wo) / bsdfSR.pdf.v;
				}
				else if (bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle)
				{
					throughput[subpath] *= fs / bsdfSR.pdf.v;
				}

				// --------------------------------------------------------------------------------

				// Setup next ray
				Ray ray;
				ray.d = bsdfSR.wo;
				ray.o = currP[subpath];
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// Intersection query
				Intersection isect;
				if (!scene.Intersect(ray, isect))
				{
					break;
				}

				// --------------------------------------------------------------------------------

				// Compute raster position if current length of eye sub-path is zero
				if (subpath == Subpath::E && pathLength[Subpath::E] == 0 && !scene.MainCamera()->RayToRasterPosition(ray.o, ray.d, rasterPos))
				{
					break;
				}

				// Update information
				currIsect[subpath] = isect;
				currP[subpath] = isect.p;
				currGN[subpath] = isect.gn;
				currWi[subpath] = -ray.d;
				pathLength[subpath]++;
			}
		}

		processedBlocks++;
		signal_ReportProgress(static_cast<double>(processedBlocks) / blocks, processedBlocks == blocks);
	}

	// --------------------------------------------------------------------------------

	// Accumulate rendered results for all threads to one film
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(f.get());
	}

	return true;
}

Math::Vec3 SimpleBidirectionalPathtraceRenderer::Impl::EvaluateGeneralizedBSDF( const GeneralizedBSDFEvaluateQuery& query ) const
{
	if (query.pathLength == 0)
	{
		if (query.subpath == Subpath::E)
		{
			// Evaluate directional component of We
			return query.camera->EvaluateDirectionalWe(query.p, query.wo);
		}
		else
		{
			// Evaluate directional component of Le
			return query.light->EvaluateDirectionalLe(query.p, query.gn, query.wo);
		}
	}
	else
	{
		// Evaluate BSDF
		BSDFEvaluateQuery bsdfEQ;
		bsdfEQ.transportDir = static_cast<TransportDirection>(query.subpath);
		bsdfEQ.type = BSDFType::All;
		bsdfEQ.wi = query.isect.worldToShading * query.wi;
		bsdfEQ.wo = query.isect.worldToShading * query.wo;
		return query.isect.primitive->bsdf->Evaluate(bsdfEQ, query.isect);
	}
}

Math::Float SimpleBidirectionalPathtraceRenderer::Impl::EvaluateGeneralizedGeometryTerm( Math::Vec3 currP[2], Math::Vec3 currGN[2], int pathLength[2] ) const
{
	// Note : we currently assume #camera is
	// - positionally degenerated
	// - directionally non-degenerated
	// and #light is
	// - positionally non-degenerated
	// - directionally non-degenerated
	// this assumption may be changed in future implementation.

	const auto& pE = currP[Subpath::E];
	const auto& pL = currP[Subpath::L];
	auto pEpL = pL - pE;
	auto pEpL_Length = Math::Length(pEpL);
	auto pEpL_Length2 = Math::Length2(pEpL);
	pEpL /= pEpL_Length;

	if (pathLength[Subpath::E] == 0)
	{
		return Math::Dot(currGN[Subpath::L], -pEpL) / pEpL_Length2;
	}
	else
	{
		return Math::Dot(currGN[Subpath::E], pEpL) * Math::Dot(currGN[Subpath::L], -pEpL) / pEpL_Length2;
	}
}

bool SimpleBidirectionalPathtraceRenderer::Impl::SampleGeneralizedBSDF( const GeneralizedBSDFSampleQuery& query, GeneralizedBSDFSampleResult& result ) const
{
	if (query.pathLength == 0)
	{
		if (query.subpath == Subpath::E)
		{
			// Sample directional component of camera
			query.camera->SampleDirection(query.sample, query.p, query.gn, result.wo, result.pdf);
		}
		else
		{
			// Sample directional component of light
			query.light->SampleDirection(query.sample, query.p, query.gn, result.wo, result.pdf);
		}
	}
	else
	{
		// Sample BSDF
		BSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = query.sample;
		bsdfSQ.type = BSDFType::All;
		bsdfSQ.transportDir = static_cast<TransportDirection>(query.subpath);
		bsdfSQ.wi = query.isect.worldToShading * query.wi;

		BSDFSampleResult bsdfSR;
		if (!query.isect.primitive->bsdf->Sample(bsdfSQ, bsdfSR))
		{
			return false;
		}

		result.wo = query.isect.shadingToWorld * bsdfSR.wo;
		result.pdf = bsdfSR.pdf;
	}

	return true;
}

// --------------------------------------------------------------------------------

SimpleBidirectionalPathtraceRenderer::SimpleBidirectionalPathtraceRenderer()
	: p(new Impl(this))
{

}

SimpleBidirectionalPathtraceRenderer::~SimpleBidirectionalPathtraceRenderer()
{
	LM_SAFE_DELETE(p);
}

bool SimpleBidirectionalPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

bool SimpleBidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	return p->Render(scene);
}

boost::signals2::connection SimpleBidirectionalPathtraceRenderer::Connect_ReportProgress( const std::function<void (double, bool ) >& func )
{
	return p->Connect_ReportProgress(func);
}

LM_NAMESPACE_END