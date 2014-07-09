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
	virtual void SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats );
	virtual void SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats );

private:

	int rrDepth;	// Depth of beginning RR

};

bool PSSMLTPTPathSampler::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
	return true;
}

PSSMLTPathSampler* PSSMLTPTPathSampler::Clone()
{
	auto* sampler = new PSSMLTPTPathSampler;
	sampler->rrDepth = rrDepth;
	return sampler;
}

void PSSMLTPTPathSampler::SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats )
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
	int depth = 0;

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

		if (++depth >= rrDepth)
		{
			// Russian roulette for path termination
			Math::Float p = Math::Min(Math::Float(0.5), Math::Luminance(throughput));
			if (sampler.Next() > p)
			{
				break;
			}

			throughput /= p;
		}
	}

	splats.splats.emplace_back(rasterPos, L);
}

void PSSMLTPTPathSampler::SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats )
{
	LM_LOG_ERROR("Invalid operation for PSSMLTPTPathSampler");
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTPTPathSampler, PSSMLTPathSampler);

LM_NAMESPACE_END