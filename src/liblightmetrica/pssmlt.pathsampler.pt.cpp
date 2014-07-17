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
#include <lightmetrica/pssmlt.pathsampler.h>
#include <lightmetrica/pssmlt.splat.h>
#include <lightmetrica/sampler.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

/*!
	Path tracing sampler.
	Implements path sampler for PSSMLT with (unidirectional) path tracing.
*/
class PSSMLTPTPathSampler : public PSSMLTPathSampler
{
public:

	LM_COMPONENT_IMPL_DEF("pt");

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual PSSMLTPathSampler* Clone();
	virtual void SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices );
	virtual void SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices );
	virtual void SampleAndEvaluateBidirSpecified( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices, int s, int t );

};

bool PSSMLTPTPathSampler::Configure( const ConfigNode& node, const Assets& assets )
{
	return true;
}

PSSMLTPathSampler* PSSMLTPTPathSampler::Clone()
{
	auto* sampler = new PSSMLTPTPathSampler;
	return sampler;
}

void PSSMLTPTPathSampler::SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices )
{
	// Clear result
	splats.splats.clear();

	// Raster position
	auto rasterPos = sampler.NextVec2();

	// Sample position on camera
	SurfaceGeometry geomE;
	Math::PDFEval pdfP;
	scene.MainCamera()->SamplePosition(sampler.NextVec2(), geomE, pdfP);

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
		if (maxPathVertices != -1 && numPathVertices > maxPathVertices)
		{
			break;
		}

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
		bsdfSQ.sample = sampler.NextVec2();
		bsdfSQ.uComp = sampler.Next();
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

		if (numPathVertices >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}

		numPathVertices++;

		if (maxPathVertices != -1 && numPathVertices >= maxPathVertices)
		{
			break;
		}
	}

	splats.splats.emplace_back(rasterPos, L);
}

void PSSMLTPTPathSampler::SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxDepth )
{
	LM_LOG_ERROR("Invalid operation for PSSMLTPTPathSampler");
}

void PSSMLTPTPathSampler::SampleAndEvaluateBidirSpecified( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices, int s, int t )
{
	LM_LOG_ERROR("Invalid operation for PSSMLTPTPathSampler");
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTPTPathSampler, PSSMLTPathSampler);

LM_NAMESPACE_END