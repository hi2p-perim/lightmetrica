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
	virtual void SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats );

private:

	int rrDepth;										//!< Depth of beginning RR
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
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);

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
	sampler->rrDepth = rrDepth;
	sampler->misWeight.reset(misWeight->Clone());
	sampler->pool.reset(new BPTPathVertexPool);
	return sampler;
}

void PSSMLTBPTPathSampler::SampleAndEvaluate( const Scene& scene, Sampler& sampler, PSSMLTSplats& splats )
{
	// Clear result
	splats.splats.clear();

	// Release and clear paths
	pool->Release();
	lightSubpath.Clear();
	eyeSubpath.Clear();

	// Sample subpaths
	lightSubpath.Sample(scene, sampler, *pool, rrDepth);
	eyeSubpath.Sample(scene, sampler, *pool, rrDepth);

	// Evaluate combination of sub-paths
	const int nL = static_cast<int>(lightSubpath.vertices.size());
	const int nE = static_cast<int>(eyeSubpath.vertices.size());
	for (int n = 2; n <= nE + nL; n++)
	{
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

LM_COMPONENT_REGISTER_IMPL(PSSMLTBPTPathSampler, PSSMLTPathSampler);

LM_NAMESPACE_END