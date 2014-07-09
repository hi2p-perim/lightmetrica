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
#ifndef LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H
#define LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H

#include "component.h"

LM_NAMESPACE_BEGIN

class ConfigNode;
class Assets;
class Scene;
class Sampler;
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
	virtual PSSMLTPathSampler* Clone() = 0;

	/*!
		Sample and evaluate light paths.
		Light path sampling strategy such as BPT might generate
		multiple light paths which contribute different raster position.
		We represent these different contributions by #PSSMLTSplats structure.
		\param scene Scene.
		\param sampler Abstract sampler.
		\param splats Evaluated pixel contributions and their raster positions.
	*/
	virtual void SampleAndEvaluate(const Scene& scene, Sampler& sampler, PSSMLTSplats& splats) = 0;

	/*!
		Sample and evaluate light paths (separated PSS version for BPT).
		Primary sample spaces is separated into two parts:
		for sampling light subpath and eye subpath,
		which increase coherency between mutations.
		\param scene Scene.
		\param lightSubpathSampler Abstract sampler for light subpath.
		\param eyeSubpathSampler Abstract sampler for eye subpath.
		\param splats Evaluated pixel contributions and their raster positions.
	*/
	virtual void SampleAndEvaluateBidir(const Scene& scene, Sampler& lightSubpathSampler, Sampler& eyeSubpathSampler, PSSMLTSplats& splats) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_PATH_SAMPLER_H