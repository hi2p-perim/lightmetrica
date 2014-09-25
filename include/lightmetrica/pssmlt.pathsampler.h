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
#ifndef LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H
#define LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H

#include "component.h"

LM_NAMESPACE_BEGIN

class ConfigNode;
class Assets;
class Scene;
class Sampler;
struct PSSMLTSplat;
struct PSSMLTSplats;

/*!
	Path sampler.
	An interface for light path samplers for PSSMLT.
*/
class PSSMLTPathSampler : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("pssmltpathsampler");

public:

	PSSMLTPathSampler() {}
	virtual ~PSSMLTPathSampler() {}

public:

	/*!
		Configure the sampler from XML element.
		\param node A XML element which consists of \a path_sampler element.
		\param assets Assets manager.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets) = 0;

	/*!
		Clone the sampler.
		\return Duplicated instance.
	*/
	virtual PSSMLTPathSampler* Clone() const = 0;

	/*!
		Sample and evaluate light paths.
		Light path sampling strategy such as BPT might generate
		multiple light paths which contribute different raster position.
		We represent these different contributions by #PSSMLTSplats structure.
		\param scene Scene.
		\param sampler Abstract sampler.
		\param splats Evaluated pixel contributions and their raster positions.
		\param rrDepth Depth to begin RR, -1 skips RR.
		\param maxPathVertices Maximum number of vertex, -1 specifies no limits.
	*/
	virtual void SampleAndEvaluate(const Scene& scene, Sampler& sampler, PSSMLTSplats& splats, int rrDepth, int maxPathVertices) = 0;

	/*!
		Sample and evaluate light paths (separated PSS version for BPT).
		Primary sample spaces is separated into two parts:
		for sampling light subpath and eye subpath,
		which increase coherency between mutations.
		\param scene Scene.
		\param subpathSamplerL Abstract sampler for light subpath.
		\param subpathSamplerE Abstract sampler for eye subpath.
		\param splats Evaluated pixel contributions and their raster positions.
		\param rrDepth Depth to begin RR, -1 skips RR.
		\param maxPathVertices Maximum number of vertex of each subpath, -1 specifies no limits.
	*/
	virtual void SampleAndEvaluateBidir(const Scene& scene, Sampler& subpathSamplerL, Sampler& subpathSamplerE, PSSMLTSplats& splats, int rrDepth, int maxPathVertices) = 0;

	/*!
		Sample and evaluate light paths (separated PSS version for BPT) for specified technique.
		A variant of #SampleAndEvaluateBidir with specified technique with #s and #t.
		\param scene Scene.
		\param subpathSamplerL Abstract sampler for light subpath.
		\param subpathSamplerE Abstract sampler for eye subpath.
		\param splat Evaluated pixel contribution and its raster position.
		\param rrDepth Depth to begin RR, -1 skips RR.
		\param maxPathVertices Maximum number of vertex of each subpath, -1 specifies no limits.
		\param s Number of light subpath vertices.
		\param t Number of eye subpath vertices.
	*/
	virtual void SampleAndEvaluateBidirSpecified(const Scene& scene, Sampler& subpathSamplerL, Sampler& subpathSamplerE, PSSMLTSplat& splat, int rrDepth, int maxPathVertices, int s, int t) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H