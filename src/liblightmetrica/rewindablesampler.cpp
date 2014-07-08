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