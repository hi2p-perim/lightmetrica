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
#include <lightmetrica/rewindablesampler.h>
#include <lightmetrica/random.h>

LM_NAMESPACE_BEGIN

class RewindableSamplerImpl : public RewindableSampler
{
public:

	LM_COMPONENT_IMPL_DEF("default");

public:

	virtual Sampler* Clone()
	{
		LM_LOG_ERROR("Invalid operator for RestorableSampler");
		return nullptr;
	}

	virtual void SetSeed( unsigned int seed )
	{
		currentIndex = 0;
		initialSeed = seed;
		rng->SetSeed(seed);
	}

	virtual Math::Float Next()
	{
		currentIndex++;
		return rng->Next();
	}

	virtual unsigned int NextUInt()
	{
		currentIndex++;
		return rng->NextUInt();
	}

	virtual Math::Vec2 NextVec2()
	{
		currentIndex += 2;
		return rng->NextVec2();
	}

	virtual Random* Rng()
	{
		return rng.get();
	}

public:

	virtual void Configure( Random* rng )
	{
		this->rng.reset(rng);
	}

	virtual void Rewind( int index )
	{
		// Reset the initial seed
		currentIndex = 0;
		rng->SetSeed(initialSeed);

		// Generate samples until the given index
		while (currentIndex < index)
		{
			// Discard the value
			currentIndex++;
			rng->Next();
		}
	}

	virtual int SampleIndex() const
	{
		return currentIndex;
	}

private:
	
	unsigned int initialSeed;		// Initial seed
	std::unique_ptr<Random> rng;	// Random number generator
	int currentIndex;				// Number of generated samples

};

LM_COMPONENT_REGISTER_IMPL(RewindableSamplerImpl, RewindableSampler);

LM_NAMESPACE_END