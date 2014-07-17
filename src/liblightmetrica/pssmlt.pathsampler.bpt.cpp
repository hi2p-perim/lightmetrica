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
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.mis.h>
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

LM_NAMESPACE_BEGIN

/*!
	Bidirectional path tracing sampler.
	Implements path sampler for PSSMLT with BPT.
*/
class PSSMLTBPTPathSampler : public PSSMLTPathSampler
{
public:

	LM_COMPONENT_IMPL_DEF("bpt");

public:

	PSSMLTBPTPathSampler();

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual PSSMLTPathSampler* Clone();
	virtual void SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices );
	virtual void SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices );
	virtual void SampleAndEvaluateBidirSpecified( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices, int s, int t );

private:

	std::unique_ptr<BPTMISWeight> misWeight;			//!< MIS weighting function

private:

	std::unique_ptr<BPTPathVertexPool> pool;			//!< Memory pool for path vertices
	BPTSubpath lightSubpath;							//!< Light subpath
	BPTSubpath eyeSubpath;								//!< Eye subpath

};

PSSMLTBPTPathSampler::PSSMLTBPTPathSampler()
	: lightSubpath(TransportDirection::LE)
	, eyeSubpath(TransportDirection::EL)
{

}

bool PSSMLTBPTPathSampler::Configure( const ConfigNode& node, const Assets& assets )
{
	auto misWeightModeNode = node.Child("mis_weight");
	if (misWeightModeNode.Empty())
	{
		LM_LOG_ERROR("Missing 'mis_weight' element");
		return false;
	}
	auto misWeightType = misWeightModeNode.AttributeValue("type");
	if (!ComponentFactory::CheckRegistered<BPTMISWeight>(misWeightType))
	{
		LM_LOG_ERROR("Unsupported MIS weighting function '" + misWeightType + "'");
		return false;
	}
	auto* p = ComponentFactory::Create<BPTMISWeight>(misWeightType);
	if (p == nullptr)
	{
		return false;
	}
	misWeight.reset(p);
	if (!p->Configure(misWeightModeNode, assets))
	{
		return false;
	}

	pool.reset(new BPTPathVertexPool);

	return true;
}

PSSMLTPathSampler* PSSMLTBPTPathSampler::Clone()
{
	auto* sampler = new PSSMLTBPTPathSampler;
	sampler->misWeight.reset(misWeight->Clone());
	sampler->pool.reset(new BPTPathVertexPool);
	return sampler;
}

void PSSMLTBPTPathSampler::SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices )
{
	SampleAndEvaluateBidir(scene, sampler, sampler, splats, rrDepth, maxPathVertices);
}

void PSSMLTBPTPathSampler::SampleAndEvaluateBidir( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices )
{
	// Clear result
	splats.splats.clear();

	// Release and clear paths
	pool->Release();
	lightSubpath.Clear();
	eyeSubpath.Clear();

	// Sample subpaths
	lightSubpath.Sample(scene, lightSubpathSampler, *pool, rrDepth, maxPathVertices);
	eyeSubpath.Sample(scene, eyeSubpathSampler, *pool, rrDepth, maxPathVertices);

	// Evaluate combination of sub-paths
	const int nL = static_cast<int>(lightSubpath.vertices.size());
	const int nE = static_cast<int>(eyeSubpath.vertices.size());
	for (int n = 2; n <= nE + nL; n++)
	{
		if (maxPathVertices != -1 && n > maxPathVertices)
		{
			continue;
		}

		const int minS = Math::Max(0, n-nE);
		const int maxS = Math::Min(nL, n);
		for (int s = minS; s <= maxS; s++)
		{
			// Create fullpath
			const int t = n - s;
			BPTFullPath fullPath(s, t, lightSubpath, eyeSubpath);

			// Evaluate unweighted contribution
			Math::Vec2 rasterPosition;
			auto Cstar = fullPath.EvaluateUnweightContribution(scene, rasterPosition);
			if (Math::IsZero(Cstar))
			{
				continue;
			}

			// Evaluate contribution and record the splat
			auto C = misWeight->Evaluate(fullPath) * Cstar;
			splats.splats.emplace_back(s, t, rasterPosition, C);
		}
	}
}

void PSSMLTBPTPathSampler::SampleAndEvaluateBidirSpecified( const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices, int s, int t )
{
	// Clear result
	splats.splats.clear();

	// Release and clear paths
	pool->Release();
	lightSubpath.Clear();
	eyeSubpath.Clear();

	// Sample subpaths
	lightSubpath.Sample(scene, lightSubpathSampler, *pool, rrDepth, maxPathVertices);
	eyeSubpath.Sample(scene, eyeSubpathSampler, *pool, rrDepth, maxPathVertices);

	// Evaluate specified sub-paths
	LM_ASSERT(s + t <= maxPathVertices);
	LM_ASSERT(s <= static_cast<int>(lightSubpath.vertices.size()));
	LM_ASSERT(t <= static_cast<int>(eyeSubpath.vertices.size()));

	// Create fullpath
	BPTFullPath fullPath(s, t, lightSubpath, eyeSubpath);

	// Evaluate unweighted contribution
	Math::Vec2 rasterPosition;
	auto Cstar = fullPath.EvaluateUnweightContribution(scene, rasterPosition);
	if (Math::IsZero(Cstar))
	{
		return;
	}

	// Evaluate contribution and record the splat
	auto C = misWeight->Evaluate(fullPath) * Cstar;
	splats.splats.emplace_back(s, t, rasterPosition, C);
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTBPTPathSampler, PSSMLTPathSampler);

LM_NAMESPACE_END