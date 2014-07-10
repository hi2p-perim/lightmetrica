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
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/configurablesampler.h>
#include <thread>

LM_NAMESPACE_BEGIN

bool BPTConfig::Load( const ConfigNode& node, const Assets& assets )
{
	// Load parameters
	node.ChildValueOrDefault("num_samples", 1LL, numSamples);
	node.ChildValueOrDefault("rr_depth", 1, rrDepth);
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

	// MIS weight function
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

#if LM_ENABLE_BPT_EXPERIMENTAL
	// Experimental parameters
	auto experimentalNode = node.Child("experimental");
	if (!experimentalNode.Empty())
	{
		enableExperimentalMode = true;
		experimentalNode.ChildValueOrDefault("max_subpath_num_vertices", 3, maxSubpathNumVertices);
		experimentalNode.ChildValueOrDefault<std::string>("subpath_image_dir", "bpt", subpathImageDir);

		// At least #maxSubpathNumVertices vertices are sampled in the experimental mode
		rrDepth = Math::Max(rrDepth, maxSubpathNumVertices);
	}
	else
	{
		enableExperimentalMode = false;
	}
#endif

	return true;
}

LM_NAMESPACE_END