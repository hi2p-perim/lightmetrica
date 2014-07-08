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
#include <lightmetrica/configurablesampler.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/random.h>

LM_NAMESPACE_BEGIN

/*!
	Random sampler.
	A sampler implementation with simple random number generation.
	This implementation simply routes random number generator.
*/
class RandomSampler : public ConfigurableSampler
{
public:

	LM_COMPONENT_IMPL_DEF("random");

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets )
	{
		// Load parameters
		std::string rngType;
		node.ChildValueOrDefault("rng", std::string("sfmt"), rngType);
		if (!ComponentFactory::CheckRegistered<Random>(rngType))
		{
			LM_LOG_ERROR("Unsupported random number generator '" + rngType + "'");
			return false;
		}

		// Seed for random number
		node.ChildValueOrDefault("rng_seed", -1, initialSeed);
		if (initialSeed < 0)
		{
			initialSeed = static_cast<int>(std::time(nullptr));
		}
		
		// Create random number generator
		rng.reset(ComponentFactory::Create<Random>(rngType));
		rng->SetSeed(initialSeed);

		return true;
	}

	virtual Sampler* Clone()
	{
		auto* sampler = new RandomSampler;
		sampler->rng.reset(ComponentFactory::Create<Random>(rng->ComponentImplTypeName()));
		sampler->SetSeed(initialSeed);
		return sampler;
	}

	virtual void SetSeed( unsigned int seed )
	{
		initialSeed = seed;
		rng->SetSeed(initialSeed);
	}

	virtual Math::Float Next()
	{
		return rng->Next();
	}

	virtual unsigned int NextUInt()
	{
		return rng->NextUInt();
	}

	virtual Math::Vec2 NextVec2()
	{
		return rng->NextVec2();
	}

	virtual Random* Rng()
	{
		return rng.get();
	}

private:

	std::unique_ptr<Random> rng;
	int initialSeed;

};

LM_COMPONENT_REGISTER_IMPL(RandomSampler, ConfigurableSampler);

LM_NAMESPACE_END
