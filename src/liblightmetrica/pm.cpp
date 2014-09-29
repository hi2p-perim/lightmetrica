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
#include <lightmetrica/pm.photon.h>
#include <lightmetrica/pm.photonmap.h>
#include <lightmetrica/pm.kernel.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/surfacegeometry.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/film.h>
#include <thread>
#include <atomic>
#include <omp.h>

LM_NAMESPACE_BEGIN

typedef std::pair<const Photon*, Math::Float> CollectedPhotonInfo;

/*!
	Photon mapping renderer.
	Implements photon mapping. Unoptimized version.
	References:
	  - H. W. Jensen, Global illumination using photon maps,
	    Procs. of the Eurographics Workshop on Rendering Techniques 96, pp.21-30, 1996.
	  - H. W. Jensen, Realistic image synthesis using photon mapping,
	    AK Peters, 2001
*/
class PhotonMappingRenderer : public Renderer
{
private:

	friend class PhotonMappingRenderer_RenderProcess;

public:

	LM_COMPONENT_IMPL_DEF("pm");

public:

	virtual std::string Type() const { return ImplTypeName(); }
	virtual bool Configure(const ConfigNode& node, const Assets& assets, const Scene& scene);
	virtual bool Preprocess(const Scene& scene);
	virtual bool Postprocess(const Scene& scene) const;
	virtual RenderProcess* CreateRenderProcess(const Scene& scene, int threadID, int numThreads);
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) { return signal_ReportProgress.connect(func); }

private:

	void TracePhotons(const Scene& scene, Photons& photons, long long& tracedPaths) const;
	void VisualizePhotons(const Scene& scene, Film& film) const;

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

private:

	long long numPhotonTraceSamples;							// Number of samples emitted in photon tracing step
	long long maxPhotons;										// Maximum number of photons stored in photon map
	int maxPhotonTraceDepth;									// Maximum depth in photon tracing step
	int numNNQueryPhotons;										// Number of photon to be collected in NN query
	Math::Float maxNNQueryDist2;								// Maximum distance between query point and photons in photon map (squared)
	std::unique_ptr<ConfigurableSampler> initialSampler;		// Sampler
	bool visualizePhotons;

private:

	std::unique_ptr<PhotonMap> photonMap;						// Photon map
	std::unique_ptr<PhotonDensityEstimationKernel> pdeKernel;	// Photon density estimation kernel
	long long tracedLightPaths;									// # of traced light paths (used for density estimation)

};

// --------------------------------------------------------------------------------

/*!
	Render process for PSSMLTRenderer.
	The class is responsible for per-thread execution of rendering tasks
	and managing thread-dependent resources.
*/
class PhotonMappingRenderer_RenderProcess : public SamplingBasedRenderProcess
{
public:

	PhotonMappingRenderer_RenderProcess(const PhotonMappingRenderer& renderer, Sampler* sampler, Film* film)
		: renderer(renderer)
		, sampler(sampler)
		, film(film)
	{
		collectedPhotonInfo.reserve(renderer.numNNQueryPhotons);
	}

private:

	LM_DISABLE_COPY_AND_MOVE(PhotonMappingRenderer_RenderProcess);

public:

	virtual void ProcessSingleSample(const Scene& scene);
	virtual const Film* GetFilm() const { return film.get(); }

private:

	const PhotonMappingRenderer& renderer;
	std::unique_ptr<Sampler> sampler;
	std::unique_ptr<Film> film;
	std::vector<CollectedPhotonInfo> collectedPhotonInfo;

};

// --------------------------------------------------------------------------------

bool PhotonMappingRenderer::Configure(const ConfigNode& node, const Assets& assets, const Scene& scene)
{
	node.ChildValueOrDefault("num_photon_trace_samples", 1LL, numPhotonTraceSamples);
	node.ChildValueOrDefault("max_photons", 1LL, maxPhotons);
	node.ChildValueOrDefault("max_photon_trace_depth", -1, maxPhotonTraceDepth);
	node.ChildValueOrDefault("num_nn_query_photons", 50, numNNQueryPhotons);

	// 'max_nn_query_dist'
	Math::Float maxNNQueryDist; 
	node.ChildValueOrDefault("max_nn_query_dist", Math::Float(0.1), maxNNQueryDist);
	maxNNQueryDist2 = maxNNQueryDist * maxNNQueryDist;

	// 'photon_map_impl'
	std::string photonMapImplType;
	node.ChildValueOrDefault("photon_map_impl", std::string("kdtree"), photonMapImplType);
	if (!ComponentFactory::CheckRegistered<PhotonMap>(photonMapImplType))
	{
		LM_LOG_ERROR("Unsupported photon map implementation '" + photonMapImplType + "'");
		return false;
	}
	photonMap.reset(ComponentFactory::Create<PhotonMap>(photonMapImplType));
	if (photonMap == nullptr)
	{
		return false;
	}

	// 'pde_kernel'
	std::string pdeKernelType;
	node.ChildValueOrDefault("pde_kernel", std::string("simpson"), pdeKernelType);
	if (!ComponentFactory::CheckRegistered<PhotonDensityEstimationKernel>(pdeKernelType))
	{
		LM_LOG_ERROR("Unsupported photon density estimation kernel type '" + pdeKernelType + "'");
		return false;
	}
	pdeKernel.reset(ComponentFactory::Create<PhotonDensityEstimationKernel>(pdeKernelType));
	if (pdeKernel == nullptr)
	{
		return false;
	}

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

	// 'experimental'
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		LM_LOG_WARN("Experimental mode is enabled")
		experimentalNode.ChildValueOrDefault("visualize_photons", false, visualizePhotons);
	}
	else
	{
		visualizePhotons = false;
	}

	return true;
}

bool PhotonMappingRenderer::Preprocess(const Scene& scene)
{
	signal_ReportProgress(0, false);

	// Photon tracing
	Photons photons;
	{
		LM_LOG_INFO("Tracing photons");
		LM_LOG_INDENTER();

		tracedLightPaths = 0;
		photons.reserve(maxPhotons);
		TracePhotons(scene, photons, tracedLightPaths);

		LM_LOG_INFO("Completed");
		LM_LOG_INFO("Traced " + std::to_string(tracedLightPaths) + " light paths");
		LM_LOG_INFO("Stored " + std::to_string(photons.size()) + " photons");
	}

	// Build photon map
	{
		LM_LOG_INFO("Building photon map");
		LM_LOG_INDENTER();

		photonMap->Build(photons);
		
		LM_LOG_INFO("Completed");
	}

	signal_ReportProgress(1, true);
	return true;
}

bool PhotonMappingRenderer::Postprocess(const Scene& scene) const
{
	// Visualize photons (for debugging)
	if (visualizePhotons)
	{
		LM_LOG_INFO("Visualizing photon map");
		VisualizePhotons(scene, *scene.MainCamera()->GetFilm());
	}

	return true;
}

RenderProcess* PhotonMappingRenderer::CreateRenderProcess(const Scene& scene, int threadID, int numThreads)
{
	auto* sampler = initialSampler->Clone();
	sampler->SetSeed(initialSampler->NextUInt());
	return new PhotonMappingRenderer_RenderProcess(*this, sampler, scene.MainCamera()->GetFilm()->Clone());
}

void PhotonMappingRenderer::VisualizePhotons(const Scene& scene, Film& film) const
{
	// Camera position
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(Math::Vec2(), geomE, pdfPE);

	// Visualize photons as points
	std::vector<const Photon*> photons;
	photonMap->GetPhotons(photons);
	for (const auto* photon : photons)
	{
		Math::Vec2 rasterPos;
		if (!scene.MainCamera()->RayToRasterPosition(geomE.p, Math::Normalize(photon->p - geomE.p), rasterPos))
		{
			continue;
		}

		film.RecordContribution(rasterPos, Math::Vec3(Math::Float(1), Math::Float(0), Math::Float(0)));
	}
}

void PhotonMappingRenderer::TracePhotons(const Scene& scene, Photons& photons, long long& tracedPaths) const
{
	std::unique_ptr<Sampler> sampler(initialSampler->Clone());
	sampler->SetSeed(initialSampler->NextUInt());
	
	for (long long sample = 0; sample < numPhotonTraceSamples && static_cast<long long>(photons.size()) < maxPhotons; sample++)
	{
		tracedPaths++;

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

		// Trace light particle and evaluate importance
		auto throughput = positionalLe / pdfPL.v;
		auto currGeom = geomL;
		Math::Vec3 currWi;
		const GeneralizedBSDF* currBsdf = light;
		int depth = 0;

		while (true)
		{
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

			auto nextThroughput = throughput * fs_Estimated;

			// Russian roulette for path termination
			if (depth >= 1)
			{
				auto continueProb = Math::Min(Math::Float(1), Math::Luminance(nextThroughput) / Math::Luminance(throughput));
				if (sampler->Next() > continueProb)
				{
					break;
				}

				throughput = nextThroughput / continueProb;
			}
			else
			{
				throughput = nextThroughput;
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

			// --------------------------------------------------------------------------------

			// If intersected surface is non-specular, store the photon into photon map
			if ((isect.primitive->bsdf->BSDFTypes() & GeneralizedBSDFType::Specular) == 0)
			{
				Photon photon;
				photon.p = isect.geom.p;
				photon.throughput = throughput;
				photon.wi = -ray.d;
				photons.push_back(photon);
				if (photons.size() == static_cast<size_t>(maxPhotons))
				{
					break;
				}
			}

			// --------------------------------------------------------------------------------

			// Update information
			currGeom = isect.geom;
			currWi = -ray.d;
			currBsdf = isect.primitive->bsdf;
			depth++;
		}
	}
}

// --------------------------------------------------------------------------------

void PhotonMappingRenderer_RenderProcess::ProcessSingleSample(const Scene& scene)
{
	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfPE;
	scene.MainCamera()->SamplePosition(sampler->NextVec2(), geomE, pdfPE);

	// Evaluate positional component of We
	auto positionalWe = scene.MainCamera()->EvaluatePosition(geomE);

	auto throughput = positionalWe / pdfPE.v;
	auto currGeom = geomE;
	Math::Vec3 currWi;
	const GeneralizedBSDF* currBsdf = scene.MainCamera();
	Math::Vec2 rasterPos;
	Math::Vec3 L;

	while (true)
	{
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

		// Compute raster position if needed
		if (currBsdf == scene.MainCamera())
		{
			if (!scene.MainCamera()->RayToRasterPosition(currGeom.p, bsdfSR.wo, rasterPos))
			{
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

		// Intersected with light
		// ES*L paths are handled separately
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

		// If intersected surface is non-specular, compute radiance from photon map
		if ((isect.primitive->bsdf->BSDFTypes() & GeneralizedBSDFType::Specular) == 0)
		{
			// Collect near photons
			Math::Float maxDist2 = renderer.maxNNQueryDist2;
			collectedPhotonInfo.clear();
			const size_t n = static_cast<size_t>(renderer.numNNQueryPhotons);
			renderer.photonMap->CollectPhotons(isect.geom.p, maxDist2, [&n, this](const Math::Vec3& p, const Photon& photon, Math::Float& maxDist2)
			{
				auto dist2 = Math::Length2(photon.p - p);
				const auto comp = [](const CollectedPhotonInfo& p1, const CollectedPhotonInfo& p2)
				{
					return p1.second < p2.second;
				};

				if (collectedPhotonInfo.size() < n)
				{
					collectedPhotonInfo.emplace_back(&photon, dist2);
					if (collectedPhotonInfo.size() == n)
					{
						// Create heap
						std::make_heap(collectedPhotonInfo.begin(), collectedPhotonInfo.end(), comp);
						maxDist2 = collectedPhotonInfo.front().second;
					}
				}
				else
				{
					// Update heap
					std::pop_heap(collectedPhotonInfo.begin(), collectedPhotonInfo.end(), comp);
					collectedPhotonInfo.back() = std::make_pair(&photon, dist2);
					std::push_heap(collectedPhotonInfo.begin(), collectedPhotonInfo.end(), comp);
					maxDist2 = collectedPhotonInfo.front().second;
				}
			});

			// Density estimation
			for (const auto& info : collectedPhotonInfo)
			{
				const auto* photon = info.first;

				// Evaluate photon density estimation kernel
				// Do not to forget to divide by #tracedLightPaths
				auto k = renderer.pdeKernel->Evaluate(isect.geom.p, *photon, maxDist2);
				auto p = k / (maxDist2 * renderer.tracedLightPaths);

				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.type = GeneralizedBSDFType::AllBSDF;
				bsdfEQ.wi = -ray.d;
				bsdfEQ.wo = photon->wi;
				auto fs = isect.primitive->bsdf->EvaluateDirection(bsdfEQ, isect.geom);
				if (Math::IsZero(fs))
				{
					continue;
				}

				L += throughput * p * fs * photon->throughput;
			}

			break;
		}

		// --------------------------------------------------------------------------------

		// Update information
		currGeom = isect.geom;
		currWi = -ray.d;
		currBsdf = isect.primitive->bsdf;
	}

	// Record to film
	if (!Math::IsZero(L))
	{
		film->AccumulateContribution(rasterPos, L);
	}
}

LM_COMPONENT_REGISTER_IMPL(PhotonMappingRenderer, Renderer);

LM_NAMESPACE_END