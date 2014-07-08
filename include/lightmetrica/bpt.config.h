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

#pragma once
#ifndef LIB_LIGHTMETRICA_BPT_CONFIG_H
#define LIB_LIGHTMETRICA_BPT_CONFIG_H

#include "bpt.common.h"
#include "math.types.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

LM_NAMESPACE_BEGIN

class ConfigNode;
class Assets;
class ConfigurableSampler;
class BPTMISWeight;

/*!
	BPT config.
	Configuration of BPTRenderer.
*/
class BPTConfig
{
public:

	/*!
		Load configuration.
		\param node Configuration node.
		\param assets Asset manager.
		\retval true Succeeded to load the configuration.
		\retval false Failed to load the configuration.
	*/
	bool Load(const ConfigNode& node, const Assets& assets);

public:

	long long numSamples;									//!< Number of samples
	int rrDepth;											//!< Depth of beginning RR
	int numThreads;											//!< Number of threads
	long long samplesPerBlock;								//!< Samples to be processed per block
	std::unique_ptr<ConfigurableSampler> initialSampler;		//!< Sampler
	std::unique_ptr<BPTMISWeight> misWeight;				//!< MIS weighting function

#if LM_ENABLE_BPT_EXPERIMENTAL
	bool enableExperimentalMode;		//!< Enables experimental mode if true
	int maxSubpathNumVertices;			//!< Maximum number of vertices of sub-paths
	std::string subpathImageDir;		//!< Output directory of sub-path images
#endif

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_CONFIG_H