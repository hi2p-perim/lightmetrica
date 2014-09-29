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
#include <lightmetrica/logger.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/film.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/light.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/defaultexperiments.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

/*!
	Light trace renderer.
	An implementation of light tracing (a.k.a. inverse path tracing, particle tracing).
	Reference:
		J. Arvo and D. Kirk, Particle transport and image synthesis,
		Computer Graphics (Procs. of SIGGRAPH 90), 24, 4, pp.63-66, 1990.
*/
class LighttraceRenderer : public Renderer
{
private:

	friend class LighttraceRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("lt");

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
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

private:

#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif

};

// --------------------------------------------------------------------------------

/*!
	Render process for PathtraceRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class LighttraceRenderer_RenderProcess : public SamplingBasedRenderProcess
{
public:

	LighttraceRenderer_RenderProcess(const LighttraceRenderer& renderer, Sampler* sampler, Film* film)
		: renderer(renderer)
		, sampler(sampler)
		, film(film)
	{

	}

private:

	LM_DISABLE_COPY_AND_MOVE(LighttraceRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

private:

	const LighttraceRenderer& renderer;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<Film> film;

};

// --------------------------------------------------------------------------------

bool LighttraceRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
{
	// Load parameters
	node.ChildValueOrDefault("rr_depth", 0, rrDepth);
	
	// Sampler
	auto samplerNode = node.Child("sampler");
	auto samplerNodeType = samplerNode.AttributeValue("type");
	if (samplerNodeType != "random")
	{
		LM_LOG_ERROR("Invalid sampler type. This renderer requires 'random' sampler");
		return false;
	}
	initialSampler.reset(ComponentFactory::Create<ConfigurableSampler>(samplerNodeType));
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

RenderProcess* LighttraceRenderer::CreateRenderProcess(const Scene& scene, int threadID, int numThreads) const
{
	auto* sampler = initialSampler->Clone();
	sampler->SetSeed(initialSampler->NextUInt());
	return new LighttraceRenderer_RenderProcess(*this, sampler, scene.MainCamera()->GetFilm()->Clone());
}

// --------------------------------------------------------------------------------

void LighttraceRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	SurfaceGeometry geomL;
	Math::PDFEval pdfPL;

	// Sample a position on the light
	auto lightSampleP = sampler->NextVec2();
	Math::PDFEval lightSelectionPdf;
	const auto* light = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
	light->SamplePosition(lightSampleP, geomL, pdfPL);
	pdfPL.v *= lightSelectionPdf.v;

	// Evaluate positional component of Le
	auto positionalLe = light->EvaluatePosition(geomL);

	// --------------------------------------------------------------------------------

	// Trace light particle and evaluate importance
	auto throughput = positionalLe / pdfPL.v;
	auto currGeom = geomL;
	Math::Vec3 currWi;
	const GeneralizedBSDF* currBsdf = light;
	int depth = 0;

	while (true)
	{
		// Skip if current BSDF is directionally degenerated
		if (!currBsdf->Degenerated())
		{
			// Sample a position on camera
			SurfaceGeometry geomE;
			Math::PDFEval pdfPE;
			scene.MainCamera()->SamplePosition(sampler->NextVec2(), geomE, pdfPE);

			// Check connectivity between #geomE.p and #currGeom.p
			auto ppE = Math::Normalize(geomE.p - currGeom.p);
			if (RenderUtils::Visible(scene, currGeom.p, geomE.p))
			{
				// Calculate raster position
				Math::Vec2 rasterPos;
				if (scene.MainCamera()->RayToRasterPosition(geomE.p, -ppE, rasterPos))
				{
					GeneralizedBSDFEvaluateQuery bsdfEQ;

					// fsL
					bsdfEQ.transportDir = TransportDirection::LE;
					bsdfEQ.type = GeneralizedBSDFType::All;
					bsdfEQ.wi = currWi;
					bsdfEQ.wo = ppE;
					auto fsL = currBsdf->EvaluateDirection(bsdfEQ, currGeom);

					// fsE
					bsdfEQ.transportDir = TransportDirection::EL;
					bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
					bsdfEQ.wo = -ppE;
					auto fsE = scene.MainCamera()->EvaluateDirection(bsdfEQ, geomE);

					// Geometry term
					auto G = RenderUtils::GeneralizedGeometryTerm(currGeom, geomE);

					// Positional component of We
					auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

					// Evaluate contribution and accumulate to film
					auto contrb = throughput * fsL * G * fsE * positionalWe / pdfPE.v;
					film->AccumulateContribution(rasterPos, contrb);
				}
			}
		}

		// --------------------------------------------------------------------------------

		if (++depth >= renderer.rrDepth)
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
		bsdfSQ.transportDir = TransportDirection::LE;
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

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
		depth++;
	}
}

LM_COMPONENT_REGISTER_IMPL(LighttraceRenderer, Renderer);

LM_NAMESPACE_END