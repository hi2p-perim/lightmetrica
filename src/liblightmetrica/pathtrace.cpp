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

LM_NAMESPACE_BEGIN

/*!
	Path trace renderer.
	An implementation of path tracing.
	Reference:
		J. T. Kajiya, The rendering equation,
		Procs. of the 13th annual conference on Computer graphics and interactive techniques, 1986,
*/
class PathtraceRenderer : public Renderer
{
private:

	friend class PathtraceRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("pt");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene);
	virtual bool Preprocess(const Scene& scene) { signal_ReportProgress(1, true); return true; }
	virtual bool Postprocess() const { return true; }
	virtual RenderProcess* CreateRenderProcess(const Scene& scene) const;
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

private:

	int rrDepth;											// Depth of beginning RR
	int maxPathVertices;									// Maximum number of light path vertices
	std::unique_ptr<ConfigurableSampler> initialSampler;	// Sampler

#if 0
#if LM_EXPERIMENTAL_MODE
	DefaultExperiments expts;	// Experiments manager
#endif
#endif

};

// --------------------------------------------------------------------------------

/*!
	Render process for PathtraceRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class PathtraceRenderer_RenderProcess : public SamplingBasedRenderProcess
{
public:

	PathtraceRenderer_RenderProcess(const PathtraceRenderer& renderer, Sampler* sampler, Film* film)
		: renderer(renderer)
		, sampler(sampler)
		, film(film)
	{

	}

private:

	LM_DISABLE_COPY_AND_MOVE(PathtraceRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

private:

	const PathtraceRenderer& renderer;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<Film> film;

};

// --------------------------------------------------------------------------------

bool PathtraceRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
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

#if 0
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
#endif

	return true;
}

RenderProcess* PathtraceRenderer::CreateRenderProcess(const Scene& scene) const
{
	auto* sampler = initialSampler->Clone();
	sampler->SetSeed(initialSampler->NextUInt());
	return new PathtraceRenderer_RenderProcess(*this, sampler, scene.MainCamera()->GetFilm()->Clone());
}

// --------------------------------------------------------------------------------

void PathtraceRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	// Raster position
	auto rasterPos = sampler->NextVec2();

	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfP;
	scene.MainCamera()->SamplePosition(sampler->NextVec2(), geomE, pdfP);

	// Sample ray direction
	GeneralizedBSDFSampleQuery bsdfSQ;
	GeneralizedBSDFSampleResult bsdfSR;
	bsdfSQ.sample = rasterPos;
	bsdfSQ.transportDir = TransportDirection::EL;
	bsdfSQ.type = GeneralizedBSDFType::EyeDirection;
	auto We_Estimated = scene.MainCamera()->SampleAndEstimateDirection(bsdfSQ, geomE, bsdfSR);

	// Construct initial ray
	Ray ray;
	ray.o = geomE.p;
	ray.d = bsdfSR.wo;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf();

	Math::Vec3 throughput = We_Estimated;
	Math::Vec3 L;
	int numPathVertices = 1;

	while (true)
	{
		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}
					
		const auto* light = isect.primitive->light;
		if (light)
		{
			// Evaluate Le
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.type = GeneralizedBSDFType::LightDirection;
			bsdfEQ.wo = -ray.d;
			auto LeD = light->EvaluateDirection(bsdfEQ, isect.geom);
			auto LeP = light->EvaluatePosition(isect.geom);
			L += throughput * LeD * LeP;
		}

		// --------------------------------------------------------------------------------

		// Sample BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = sampler->NextVec2();
		bsdfSQ.uComp = sampler->Next();
		bsdfSQ.type = GeneralizedBSDFType::AllBSDF;
		bsdfSQ.transportDir = TransportDirection::EL;
		bsdfSQ.wi = -ray.d;
		
		GeneralizedBSDFSampleResult bsdfSR;
		auto fs_Estimated = isect.primitive->bsdf->SampleAndEstimateDirection(bsdfSQ, isect.geom, bsdfSR);
		if (Math::IsZero(fs_Estimated))
		{
			break;
		}

		// Update throughput
		throughput *= fs_Estimated;

		// Setup next ray
		ray.d = bsdfSR.wo;
		ray.o = isect.geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

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

		numPathVertices++;

		if (renderer.maxPathVertices != -1 && numPathVertices >= renderer.maxPathVertices)
		{
			break;
		}
	}

	film->AccumulateContribution(rasterPos, L);
}

LM_COMPONENT_REGISTER_IMPL(PathtraceRenderer, Renderer);

LM_NAMESPACE_END
