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
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/component.h>
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
	node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
	if (!ComponentFactory::CheckRegistered(rngType))
	{
		LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
		return false;
	}

	// MIS weight function
	// TODO : Check if valid interface for the given type with CheckRegistered
	auto misWeightModeNode = node.Child("mis_weight");
	if (misWeightModeNode.Empty())
	{
		LM_LOG_ERROR("Missing 'mis_weight' element");
		return false;
	}
	auto misWeightType = misWeightModeNode.AttributeValue("type");
	if (!ComponentFactory::CheckRegistered(misWeightType))
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
	}
	else
	{
		enableExperimentalMode = false;
	}
#endif

	return true;
}

LM_NAMESPACE_END