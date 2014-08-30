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
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/light.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexperiments.h>
#include <lightmetrica/renderutils.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Path tracing with multiple importance sampling.
	Implements path tracing using multiple importance sampling with
	BSDF sampling and direct light sampling.
*/
class MISPathtraceRenderer : public Renderer
{
public:

	LM_COMPONENT_IMPL_DEF("pt.mis");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void SetTerminationMode( RendererTerminationMode mode, double time ) {}
	virtual bool Preprocess( const Scene& scene ) { signal_ReportProgress(0, true); return true; }
	virtual bool Render( const Scene& scene );
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	void ProcessRenderSingleSample(const Scene& scene, Sampler& sampler, Film& film) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	long long numSamples;									// Number of samples
	int rrDepth;											// Depth of beginning RR
	int maxPathVertices;									// Maximum number of light path vertices
	int numThreads;											// Number of threads
	long long samplesPerBlock;								// Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

bool MISPathtraceRenderer::Configure( const ConfigNode& node, const Assets& assets )
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

	// Sampler
	auto samplerNode = node.Child("sampler");
	initialSampler.reset(ComponentFactory::Create<ConfigurableSampler>(samplerNode.AttributeValue("type")));
	if (initialSampler == nullptr || !initialSampler->Configure(samplerNode, assets))
	{
		LM_LOG_ERROR("Invalid sampler");
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

bool MISPathtraceRenderer::Render( const Scene& scene )
{
	auto* masterFilm = scene.MainCamera()->GetFilm();
	std::atomic<long long> processedBlocks(0);

	signal_ReportProgress(0, false);

	LM_EXPT_NOTIFY(expts, "RenderStarted");

	// --------------------------------------------------------------------------------

	// Set number of threads
	omp_set_num_threads(numThreads);

	// Random number generators and films
	std::vector<std::unique_ptr<Sampler>> samplers;
	std::vector<std::unique_ptr<Film>> films;
	for (int i = 0; i < numThreads; i++)
	{
		samplers.emplace_back(initialSampler->Clone());
		samplers.back()->SetSeed(initialSampler->NextUInt());
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
		auto& sampler = samplers[threadId];
		auto& film = films[threadId];

		// Sample range
		long long sampleBegin = samplesPerBlock * block;
		long long sampleEnd = Math::Min(sampleBegin + samplesPerBlock, numSamples);

		LM_EXPT_UPDATE_PARAM(expts, "film", film.get());

		for (long long sample = sampleBegin; sample < sampleEnd; sample++)
		{
			ProcessRenderSingleSample(scene, *sampler, *film);

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

void MISPathtraceRenderer::ProcessRenderSingleSample( const Scene& scene, Sampler& sampler, Film& film ) const
{
	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfPE);

	// Evaluate positional component of We
	auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

	// Trace ray from camera
	auto throughput = positionalWe / pdfPE.v;
	auto currGeom = geomE;
	Math::Vec3 currWi;
	const GeneralizedBSDF* currBsdf = scene.MainCamera();
	int numPathVertices = 1;
	Math::Vec2 rasterPos;

	while (true)
	{
		// Skip if current BSDF is directionally degenerated
		if (!currBsdf->Degenerated())
		{
			// Sample a position on light
			SurfaceGeometry geomL;
			Math::PDFEval pdfPL;
			auto lightSampleP = sampler.NextVec2();
			Math::PDFEval lightSelectionPdf;
			const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
			light->SamplePosition(lightSampleP, geomL, pdfPL);
			pdfPL.v *= lightSelectionPdf.v;

			// Check connectivity between #currGeom.p and #geomL.p  
			auto ppL = Math::Normalize(geomL.p - currGeom.p);
			if (RenderUtils::Visible(scene, currGeom.p, geomL.p))
			{
				// Calculate raster position if required
				bool visible = true;
				if (numPathVertices == 1)
				{
					visible = scene.MainCamera()->RayToRasterPosition(currGeom.p, ppL, rasterPos);
				}

				if (visible)
				{
					GeneralizedBSDFEvaluateQuery bsdfEQ;

					// fsE
					bsdfEQ.transportDir = TransportDirection::EL;
					bsdfEQ.type = GeneralizedBSDFType::All;
					bsdfEQ.wi = currWi;
					bsdfEQ.wo = ppL;
					auto fsE = currBsdf->EvaluateDirection(bsdfEQ, currGeom);

					// fsL
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::LightDirection;
					bsdfEQ.wo = -ppL;
					auto fsL = light->EvaluateDirection(bsdfEQ, geomL);

					// Positional component of Le
					auto positionalLe = light->EvaluatePosition(geomL);

					// Geometry term
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomL);

					if (!Math::IsZero(G))
					{
						// PDF for direct light sampling (in projected solid angle measure)
						auto pdfD_DirectLight = pdfPL.v / G;
						LM_ASSERT(pdfD_DirectLight > Math::Float(0));

						// PDF for BSDF sampling (in projected solid angle measure)
						bsdfEQ.transportDir = TransportDirection::EL;
						bsdfEQ.type = GeneralizedBSDFType::All;
						bsdfEQ.wi = currWi;
						bsdfEQ.wo = ppL;
						auto pdfD_BSDF = currBsdf->EvaluateDirectionPDF(bsdfEQ, currGeom).v;

						// MIS weight for direct light sampling
						auto w = pdfD_DirectLight / (pdfD_DirectLight + pdfD_BSDF);

						// Evaluate contribution and accumulate to film
						auto contrb = w * throughput * fsE * G * fsL * positionalLe / pdfPL.v;
						film.AccumulateContribution(rasterPos, contrb);
					}
				}
			}
		}

		// --------------------------------------------------------------------------------

		if (rrDepth != -1 && numPathVertices >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.uComp = sampler.Next();
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = currWi;

		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = currBsdf->SampleAndEstimateDirection(bsdfSQ, currGeom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;

		// Calculate raster position if the depth is one
		if (numPathVertices == 1)
		{
			if (!scene.MainCamera()->RayToRasterPosition(currGeom.p, bsdfSR.wo, rasterPos))
			{
				// Should not be here
				LM_ASSERT(false);
				break;
			}
		}

		// --------------------------------------------------------------------------------

		// Setup next ray
		Ray ray;
		ray.d = bsdfSR.wo;
		ray.o = currGeom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Intersection query
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// Intersected point is light
		{
			const auto* light = isect.primitive->light;
			if (light != nullptr)
			{
				// Evaluate Le
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::LE;
				bsdfEQ.type = GeneralizedBSDFType::LightDirection;
				bsdfEQ.wo = -ray.d;
				auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
				auto LeP = light->EvaluatePosition(isect.geom);

				if ((bsdfSR.sampledType & GeneralizedBSDFType::Specular) > 0)
				{
					// Previous BSDF is specular
					// There is no probability that direct light sampling
					// generate #bsdfSR.wo, so use only BSDF sampling
					film.AccumulateContribution(rasterPos, throughput * LeD * LeP);
				}
				else
				{
					// PDF for direct light sampling
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, isect.geom);
					auto pdfD_DirectLight = Math::IsZero(G) ? Math::Float(0) : scene.LightSelectionPdf().v * light->EvaluatePositionPDF(isect.geom).v / G;

					// MIS weight
					auto w = bsdfSR.pdf.v / (bsdfSR.pdf.v + pdfD_DirectLight);

					// Evaluate contribution and accumulate to film
					auto contrb = w * throughput * LeD * LeP;
					film.AccumulateContribution(rasterPos, contrb);
				}
			}
		}

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
		numPathVertices++;

		if (maxPathVertices != -1 && numPathVertices >= maxPathVertices)
		{
			break;
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(MISPathtraceRenderer, Renderer);

LM_NAMESPACE_END
