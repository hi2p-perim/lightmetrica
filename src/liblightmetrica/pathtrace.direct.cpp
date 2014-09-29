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
#include <lightmetrica/renderproc.h>
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
	Path tracing with direct light sampling.
	Implements path tracing with direct light sampling (a.k.a. next event estimation).
	In this implementation,
	E{D,S}D+L paths are sampled by direct light sampling and
	E{D,S}*S+L paths are sampled by BSDF sampling.
*/
class DirectPathtraceRenderer : public Renderer
{
private:

	friend class DirectPathtraceRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("pt.direct");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene);
	virtual bool Preprocess(const Scene& scene) { signal_ReportProgress(1, true); return true; }
	virtual bool Postprocess(const Scene& scene) const { return true; }
	virtual RenderProcess* CreateRenderProcess(const Scene& scene, int threadID, int numThreads) const;
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

private:

	int rrDepth;											// Depth of beginning RR
	int maxPathVertices;									// Maximum number of light path vertices
	long long samplesPerBlock;								// Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

private:

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

// --------------------------------------------------------------------------------

/*!
	Render process for DirectPathtraceRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class DirectPathtraceRenderer_RenderProcess : public SamplingBasedRenderProcess
{
public:

	DirectPathtraceRenderer_RenderProcess(const DirectPathtraceRenderer& renderer, Sampler* sampler, Film* film)
		: renderer(renderer)
		, sampler(sampler)
		, film(film)
	{

	}

private:

	LM_DISABLE_COPY_AND_MOVE(DirectPathtraceRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

private:

	const DirectPathtraceRenderer& renderer;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<Film> film;

};

// --------------------------------------------------------------------------------

bool DirectPathtraceRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
{
	// Load parameters
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	node.ChildValueOrDefault("max_path_vertices", -1, maxPathVertices);

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
	}
#endif

	return true;
}

RenderProcess* DirectPathtraceRenderer::CreateRenderProcess(const Scene& scene, int threadID, int numThreads) const
{
	auto* sampler = initialSampler->Clone();
	sampler->SetSeed(initialSampler->NextUInt());
	return new DirectPathtraceRenderer_RenderProcess(*this, sampler, scene.MainCamera()->GetFilm()->Clone());
}

// --------------------------------------------------------------------------------

void DirectPathtraceRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(sampler->NextVec2(), geomE, pdfPE);

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
			auto lightSampleP = sampler->NextVec2();
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

					// Geometry term
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomL);

					// Positional component of Le
					auto positionalLe = light->EvaluatePosition(geomL);

					// Evaluate contribution and accumulate to film
					auto contrb = throughput * fsE * G * fsL * positionalLe / pdfPL.v;
					film->AccumulateContribution(rasterPos, contrb);
				}
			}
		}

		// --------------------------------------------------------------------------------

		if (renderer.rrDepth != -1 && numPathVertices >= renderer.rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler->Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler->NextVec2();
		bsdfSQ.uComp = sampler->Next();
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
				// Previous BSDF is specular 
				if ((bsdfSR.sampledType & GeneralizedBSDFType::Specular) > 0)
				{
					// Evaluate Le
					GeneralizedBSDFEvaluateQuery bsdfEQ;
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::LightDirection;
					bsdfEQ.wo = -ray.d;
					auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
					auto LeP = light->EvaluatePosition(isect.geom);
					film->AccumulateContribution(rasterPos, throughput * LeD * LeP);
				}
			}
		}

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
		numPathVertices++;

		if (renderer.maxPathVertices != -1 && numPathVertices >= renderer.maxPathVertices)
		{
			break;
		}
	}
}

LM_COMPONENT_REGISTER_IMPL(DirectPathtraceRenderer, Renderer);

LM_NAMESPACE_END
