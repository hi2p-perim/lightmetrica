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

class SimpleBidirectionalPathtraceRenderer::Impl : public Object
{
public:

	//! Identifier of sub-paths.
	enum Subpath
	{
		E = 0,		//!< Eye sub-path.
		L = 1		//!< Light sub-path.
	};

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
	Math::Vec3 EvaluateGeneralizedBSDF(int subpath, Math::Vec3 currP[2], Math::Vec3 prevP[2], int pathLength[2], Intersection currIsect[2], const Light* camera, const Light* light) const;

private:

	SimpleBidirectionalPathtraceRenderer* self;
	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	int numSamples;			// Number of samples
	int rrDepth;			// Depth of beginning RR
	int numThreads;			// Number of threads
	int samplesPerBlock;	// Samples to be processed per block

};

bool SimpleBidirectionalPathtraceRenderer::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	// Check type
	if (node.AttributeValue("type") != self->Type())
	{
		LM_LOG_ERROR("Invalid renderer type '" + node.AttributeValue("type") + "'");
		return false;
	}

	// Load parameters
	node.ChildValueOrDefault("num_samples", 1, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("num_threads", static_cast<int>(std::thread::hardware_concurrency()), numThreads);
	if (numThreads <= 0)
	{
		numThreads = Math::Max(1, static_cast<int>(std::thread::hardware_concurrency()) + numThreads);
	}
	node.ChildValueOrDefault("samples_per_block", 100, samplesPerBlock);
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
	std::atomic<int> processedBlocks(0);

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
	int blocks = (numSamples + samplesPerBlock) / samplesPerBlock;

	// --------------------------------------------------------------------------------

	#pragma omp parallel for
	for (int block = 0; block < blocks; block++)
	{
		// Thread ID
		int threadId = omp_get_thread_num();
		auto& rng = rngs[threadId];
		auto& film = films[threadId];

		// Sample range
		int sampleBegin = samplesPerBlock * block;
		int sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		for (int sample = sampleBegin; sample < sampleEnd; sample++)
		{
			Math::Vec3 pE;
			Math::PDFEval pdfPE;

			// Sample position on the camera
			scene.MainCamera()->SamplePosition(rng->NextVec2(), pE, pdfPE);
			
			// Evaluate We^{(0)} (positional component of We)
			auto positionalWe = scene.MainCamera()->EvaluatePositionalWe(pE);

			// --------------------------------------------------------------------------------

			Math::Vec3 pL;
			Math::PDFEval pdfPL;

			// Sample a position on the light
			auto lightSampleP = rng->NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, pE, pdfPL);
			pdfPL.v /= lightSelectionPdf.v;

			// Evaluate Le^{(0)} (positional component of Le)
			auto positionalLe = light->EvaluatePositionalLe(pL);

			// --------------------------------------------------------------------------------

			// Length of eye and light sub-paths respectively
			int pathLength[2] = { 0 };

			// Path throughputs
			Math::Vec3 throughput[2] = { positionalWe / pdfPE.v, positionalLe / pdfPL.v };

			// Current intersection structures
			Intersection currIsect[2];

			// Current and previous positions
			Math::Vec3 currP[2] = { pE, pL };
			Math::Vec3 prevP[2];

			// Flag of zero contribution
			bool zeroContrb = false;

			while (true)
			{
				// Two sub-paths increase its length alternatively
				bool lengthChanged[2] = { 0 };
				for (int subpath = 0; subpath < 2; subpath++)
				{
					// Check connectivity between #pE and #pL
					Ray shadowRay;
					auto pEpL = currIsect[Subpath::L].p - currIsect[Subpath::E].p;
					auto pEpL_Length = Math::Length(pEpL);
					shadowRay.o = pE;
					shadowRay.d = pEpL / pEpL_Length;
					shadowRay.minT = Math::Constants::Eps();
					shadowRay.maxT = pEpL_Length * (Math::Float(1) - Math::Constants::Eps());

					Intersection shadowIsect;
					if (!scene.Intersect(shadowRay, shadowIsect))
					{
						// Generalized BSDFs
						auto fsE = EvaluateGeneralizedBSDF(Subpath::E, pathLength, currIsect, light);
						auto fsL = EvaluateGeneralizedBSDF(Subpath::L, pathLength, currIsect, light);

						// Geometry term
						auto G = EvaluateGeneralizedGeometryTerm();

						// Evaluate contribution and accumulate to film
						auto contrb = throughput[Subpath::E] * fsE * G * fsL * throughput[Subpath::L];
						film->AccumulateContribution(rasterPos, contrb * Math::Float(film->Width() * film->Height()) / Math::Float(numSamples));
					}

					// --------------------------------------------------------------------------------

					// Russian roulette
					if (pathLength[subpath] >= rrDepth)
					{
						auto p = Math::Min(Math::Float(0.5), Math::Luminance(throughput[subpath]));
						if (rng->Next() > p)
						{
							lengthChanged[subpath] = false;
							throughput[subpath] /= Math::Float(1) - p;
							continue;
						}
						else
						{
							lengthChanged[subpath] = true;
							throughput[subpath] /= p;
						}
					}

					// --------------------------------------------------------------------------------

					// Sample generalized BSDF
					Ray ray;
					if (!SampleGeneralizedBSDF())
					{
						zeroContrb = true;
						break;
					}
					
					// Evaluate generalized BSDF
					auto fs = EvaluateGeneralizedBSDF();
					
					// Update throughput
					throughput[subpath] *= fs * Math::CosThetaZUp(wo) / bsdfPdf.pdf.v;

					// --------------------------------------------------------------------------------

					// Intersection query
					Intersection isect;
					if (!scene.Intersect(ray, isect))
					{
						zeroContrb = true;
						break;
					}

					// Update #pE or #pL and intersection structure
					currIsect[subpath] = isect;
					prevP[subpath] = currP[subpath];
					currP[subpath] = isect.p;
					pathLength[subpath]++;
				}

				// Terminates if the lengths of the sub-paths do not change
				// or the path with zero contribution is generated
				if (zeroContrb || (!lengthChanged[Subpath::E] && !lengthChanged[Subpath::L]))
				{
					break;
				}
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

Math::Vec3 SimpleBidirectionalPathtraceRenderer::Impl::EvaluateGeneralizedBSDF( int subpath, Math::Vec3 currP[2], Math::Vec3 prevP[2], int pathLength[2], Intersection currIsect[2], const Light* camera, const Light* light ) const
{
	// Current and previous positions
	// Order : ppE -> pE -> pL -> ppL
	const auto& pE = currP[Subpath::E];
	const auto& pL = currP[Subpath::L];
	const auto& ppE = prevP[Subpath::E];
	const auto& ppL = prevP[Subpath::L];

	// Intersections
	const auto& isectE = currIsect[Subpath::E];
	const auto& isectL = currIsect[Subpath::L];

	if (subpath == Subpath::E)
	{
		// Query for eye sub-path
		if (pathLength[Subpath::E] == 0)
		{
			// Evaluate directional component of We
			return camera->EvaluateDirectionalWe(pE, Math::Normalize(pL - pE));
		}
		else
		{
			// Evaluate BSDF
			BSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::CameraToLight;
			bsdfEQ.type = BSDFType::All;
			bsdfEQ.wi = isectE.worldToShading * Math::Normalize(ppE - pE);
			bsdfEQ.wo = isectE.worldToShading * Math::Normalize(pL - pE);
			return isectE.primitive->bsdf->Evaluate(bsdfEQ, isectE);
		}
	}
	else
	{
		// Query for light sub-path
		if (pathLength[Subpath::L] == 0)
		{
			// Evaluate directional component of Le
			return light->EvaluateDirectionalLe();
		}
		else
		{

		}
	}
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