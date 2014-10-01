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
class RandomSampler final : public ConfigurableSampler
{
public:

	LM_COMPONENT_IMPL_DEF("random");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets) override
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

	virtual Sampler* Clone() const override
	{
		auto* sampler = new RandomSampler;
		sampler->rng.reset(ComponentFactory::Create<Random>(rng->ComponentImplTypeName()));
		sampler->SetSeed(initialSeed);
		return sampler;
	}

	virtual void SetSeed(unsigned int seed) override
	{
		initialSeed = seed;
		rng->SetSeed(initialSeed);
	}

	virtual Math::Float Next() override
	{
		return rng->Next();
	}

	virtual unsigned int NextUInt() override
	{
		return rng->NextUInt();
	}

	virtual Math::Vec2 NextVec2() override
	{
		return rng->NextVec2();
	}

	virtual Random* Rng() override
	{
		return rng.get();
	}

private:

	std::unique_ptr<Random> rng;
	int initialSeed;

};

LM_COMPONENT_REGISTER_IMPL(RandomSampler, ConfigurableSampler);

LM_NAMESPACE_END
