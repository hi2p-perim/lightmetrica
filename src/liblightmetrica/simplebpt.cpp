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
#include <lightmetrica/assert.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/defaultexpts.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Simple bidirectional path trace renderer.
	An implementation of bidirectional path tracing (BPT).
	This simple implementation omits multiple importance sampling between paths
	which is originally referred in the Veach's thesis.
	NOTE : Incorrect method. It cannot handle specular materials
*/
class SimpleBidirectionalPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("simplebpt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual bool Preprocess( const Scene& /*scene*/ ) { signal_ReportProgress(0, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;			// Number of samples
	int rrDepth;					// Depth of beginning RR
	int numThreads;					// Number of threads
	long long samplesPerBlock;		// Samples to be processed per block
	std::string rngType;			// Type of random number generator

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool SimpleBidirectionalPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
{
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

bool SimpleBidirectionalPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Random>> rngs;
	std::vector<std::unique_ptr<Film>> films;
	int seed = static_cast<int>(std::time(nullptr));
	for (int i = 0; i < numThreads; i++)
	{
		rngs.emplace_back(ComponentFactory::Create<Random>(rngType));
		rngs.back()->SetSeed(seed + i);
		films.emplace_back(masterFilm->Clone());
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

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			SurfaceGeometry geomE;
			Math::PDFEval pdfPE;

			// Sample position on the camera
			scene.MainCamera()->SamplePosition(rng->NextVec2(), geomE, pdfPE);
			
			// Evaluate We^{(0)} (positional component of We)
			auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

			// --------------------------------------------------------------------------------

			SurfaceGeometry geomL;
			Math::PDFEval pdfPL;

			// Sample a position on the light
			auto lightSampleP = rng->NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, geomL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Evaluate Le^{(0)} (positional component of Le)
			auto positionalLe = light->EvaluatePosition(geomL);

			// --------------------------------------------------------------------------------

			// Length of eye and light sub-paths respectively
			int numSubpathVertices[2];
			numSubpathVertices[TransportDirection::EL] = 1;
			numSubpathVertices[TransportDirection::LE] = 1;

			// Path throughputs
			Math::Vec3 throughput[2];
			throughput[TransportDirection::EL] = positionalWe / pdfPE.v;
			throughput[TransportDirection::LE] = positionalLe / pdfPL.v;

			// Current generalized BSDF
			const GeneralizedBSDF* currBsdfs[2];
			currBsdfs[TransportDirection::EL] = scene.MainCamera();
			currBsdfs[TransportDirection::LE] = light;

			// Current and previous positions and geometry normals
			SurfaceGeometry currGeom[2];
			currGeom[TransportDirection::EL] = geomE;
			currGeom[TransportDirection::LE] = geomL;
			
			// --------------------------------------------------------------------------------

			Math::Vec3 currWi[2];
			Math::Vec2 rasterPos;

			while (true)
			{
				if (!currBsdfs[TransportDirection::LE]->Degenerated() && !currBsdfs[TransportDirection::EL]->Degenerated())
				{
					// Check connectivity between #pE and #pL
					auto pEpL = Math::Normalize(currGeom[TransportDirection::LE].p - currGeom[TransportDirection::EL].p);
					if (RenderUtils::Visible(scene, currGeom[TransportDirection::EL].p, currGeom[TransportDirection::LE].p))
					{
						bool visible = true;
						if (numSubpathVertices[TransportDirection::EL] == 1)
						{
							// If the length of eye sub-path is zero, compute raster position here
							visible = scene.MainCamera()->RayToRasterPosition(currGeom[TransportDirection::EL].p, pEpL, rasterPos);
						}

						// If #rasterPos is visible in the screen ..
						if (visible)
						{
							GeneralizedBSDFEvaluateQuery bsdfEQ;

							// fsE
							bsdfEQ.transportDir = TransportDirection::EL;
							bsdfEQ.type = GeneralizedBSDFType::All;
							bsdfEQ.wi = currWi[TransportDirection::EL];
							bsdfEQ.wo = pEpL;
							auto fsE = currBsdfs[TransportDirection::EL]->EvaluateDirection(bsdfEQ, currGeom[TransportDirection::EL]);

							// fsL
							bsdfEQ.transportDir = TransportDirection::LE;
							bsdfEQ.type = GeneralizedBSDFType::All;
							bsdfEQ.wi = currWi[TransportDirection::LE];
							bsdfEQ.wo = -pEpL;
							auto fsL = currBsdfs[TransportDirection::LE]->EvaluateDirection(bsdfEQ, currGeom[TransportDirection::LE]);

							// Geometry term
							auto G = RenderUtils::GeneralizedGeometryTerm(currGeom[TransportDirection::EL], currGeom[TransportDirection::LE]);

							// Evaluate contribution and accumulate to film
							auto contrb = throughput[TransportDirection::EL] * fsE * G * fsL * throughput[TransportDirection::LE];
							film->AccumulateContribution(rasterPos, contrb);
						}
					}
				}

				// --------------------------------------------------------------------------------

				// Select which sub-path to be extended
				// This selection might be tricky. TODO : Add some explanation
				int subpath = rng->Next() < Math::Float(0.5) ? TransportDirection::EL : TransportDirection::LE;

				// Decide if the selected -path is actually extended by Russian roulette
				// If the sub-path is not extended, terminate the loop.
				if (numSubpathVertices[subpath] >= rrDepth)
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
				bsdfSQ.uComp = rng->Next();
				bsdfSQ.transportDir = (TransportDirection)subpath;
				bsdfSQ.type = GeneralizedBSDFType::All;
				bsdfSQ.wi = currWi[subpath];

#if 1
				GeneralizedBSDFSampleResult bsdfSR;
				auto fs_Estimated = currBsdfs[subpath]->SampleAndEstimateDirection(bsdfSQ, currGeom[subpath], bsdfSR);
				if (Math::IsZero(fs_Estimated))
				{
					break;
				}

				// Update throughput
				throughput[subpath] *= fs_Estimated;
#else
				GeneralizedBSDFSampleResult bsdfSR;
				if (!currBsdfs[subpath]->SampleDirection(bsdfSQ, currGeom[subpath], bsdfSR))
				{
					break;
				}
					
				// Evaluate generalized BSDF
				auto fs = currBsdfs[subpath]->EvaluateDirection(GeneralizedBSDFEvaluateQuery(bsdfSQ, bsdfSR), currGeom[subpath]);
				if (Math::IsZero(fs))
				{
					break;
				}

				// Update throughput
				LM_ASSERT(bsdfSR.pdf.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
				throughput[subpath] *= fs / bsdfSR.pdf.v;
#endif

				// --------------------------------------------------------------------------------
				
				// Setup next ray
				Ray ray;
				ray.d = bsdfSR.wo;
				ray.o = currGeom[subpath].p;
				ray.minT = Math::Constants::Eps();
				ray.maxT = Math::Constants::Inf();

				// Intersection query
				Intersection isect;
				if (!scene.Intersect(ray, isect))
				{
					break;
				}

				// Compute raster position if current length of eye sub-path is zero
				if (subpath == TransportDirection::EL && numSubpathVertices[TransportDirection::EL] == 1 && !scene.MainCamera()->RayToRasterPosition(ray.o, ray.d, rasterPos))
				{
					break;
				}

				// --------------------------------------------------------------------------------

				// Update information
				numSubpathVertices[subpath]++;
				currGeom[subpath] = isect.geom;
				currWi[subpath] = -ray.d;
				currBsdfs[subpath] = isect.primitive->bsdf;
			}

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
	for (auto& f : films)
	{
		masterFilm->AccumulateContribution(*f.get());
	}

	// Rescale master film
	masterFilm->Rescale(Math::Float(masterFilm->Width() * masterFilm->Height()) / Math::Float(numSamples));

	LM_EXPT_NOTIFY(expts, "RenderFinished");

	return true;
}

LM_COMPONENT_REGISTER_IMPL(SimpleBidirectionalPathtraceRenderer, Renderer);

LM_NAMESPACE_END