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
#ifndef __LIB_LIGHTMETRICA_DAGPT_SAMPLER_H__
#define __LIB_LIGHTMETRICA_DAGPT_SAMPLER_H__

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

class Scene;
class Random;
class DAGPTMemoryPool;
class DAGPTLightTransportDAG;

/*!
	Light transport DAG sampler.
	Samples possible light paths as a DAG.
*/
class LM_PUBLIC_API DAGPTLightTransportDAGSampler
{
public:

	DAGPTLightTransportDAGSampler() {}
	virtual ~DAGPTLightTransportDAGSampler() {}

private:

	LM_DISABLE_COPY_AND_MOVE(DAGPTLightTransportDAGSampler);

public:
	
	/*!
		Type of the sampler.
		\return Type in string.
	*/
	virtual std::string Type() const = 0;

	/*!
		Sample a light transport DAG.
		\param scene Scene.
		\param rng Random number generator.
		\param pool Memory pool.
		\param dag Sampled light transport DAG.
	*/
	virtual void Sample(const Scene& scene, Random& rng, DAGPTMemoryPool& pool, DAGPTLightTransportDAG& dag) const = 0;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_DAGPT_SAMPLER_H__