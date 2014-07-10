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
	std::unique_ptr<ConfigurableSampler> initialSampler;	//!< Sampler
	std::unique_ptr<BPTMISWeight> misWeight;				//!< MIS weighting function

#if LM_ENABLE_BPT_EXPERIMENTAL
	bool enableExperimentalMode;		//!< Enables experimental mode if true
	int maxSubpathNumVertices;			//!< Maximum number of vertices of sub-paths
	std::string subpathImageDir;		//!< Output directory of sub-path images
#endif

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_CONFIG_H