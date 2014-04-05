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

#include <lightmetrica/pssmlt.sampler.h>

LM_NAMESPACE_BEGIN

LM_PUBLIC_API PSSMLTRestorableSampler::PSSMLTRestorableSampler( Random* rng, unsigned int seed )
	: initialSeed(seed)
	, rng(rng)
	, currentIndex(0)
{
	rng->SetSeed(seed);
}

LM_PUBLIC_API PSSMLTRestorableSampler::PSSMLTRestorableSampler( Random* rng, const PSSMLTRestorableSampler& sampler )
	: initialSeed(sampler.initialSeed)
	, rng(rng)
	, currentIndex(0)
{
	rng->SetSeed(sampler.initialSeed);
}

LM_PUBLIC_API Math::Float PSSMLTRestorableSampler::Next()
{
	currentIndex++;
	return rng->Next();
}

LM_PUBLIC_API Random* PSSMLTRestorableSampler::Rng()
{
	return rng.get();
}

LM_PUBLIC_API void PSSMLTRestorableSampler::SetIndex( int index )
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

LM_PUBLIC_API int PSSMLTRestorableSampler::Index()
{
	return currentIndex;
}

// --------------------------------------------------------------------------------

LM_PUBLIC_API PSSMLTPrimarySample::PSSMLTPrimarySample( const Math::Float& s1, const Math::Float& s2 )
	: s1(s1)
	, s2(s2)
{
	logRatio = -Math::Log(s2 / s1);
	time = 0;
	largeStepTime = 0;
	largeStep = false;
	currentIndex = 0;
}

LM_PUBLIC_API Math::Float PSSMLTPrimarySample::Next()
{
	return PrimarySample(currentIndex++);
}

LM_PUBLIC_API Random* PSSMLTPrimarySample::Rng()
{
	return rng;
}

LM_PUBLIC_API void PSSMLTPrimarySample::Accept()
{
	if (largeStep)
	{
		// Update large step time
		largeStepTime = time;
	}

	time++;
	prevSamples.clear();
	currentIndex = 0;
}

LM_PUBLIC_API void PSSMLTPrimarySample::Reject()
{
	// Restore samples
	for (auto& v : prevSamples)
	{
		int i = std::get<0>(v);
		auto& prevSample = std::get<1>(v);
		u[i] = prevSample;
	}

	prevSamples.clear();
	currentIndex = 0;
}

Math::Float PSSMLTPrimarySample::PrimarySample( int i )
{
	// Not sampled yet
	while (i >= (int)u.size())
	{
		u.push_back(Sample(rng->Next()));
	}

	// If the modified time of the requested sample is not updated
	// it requires the lazy evaluation of mutations.
	if (u[i].modify < time)
	{
		if (largeStep)
		{
			// Large step case

			// Save sample in order to restore previous state
			prevSamples.emplace_back(i, u[i]);

			// Update the modified time and value
			u[i].modify = time;
			u[i].value = rng->Next();
		}
		else
		{
			// Small step case

			// If the modified time is not updated since the last accepted
			// large step mutation, then update sample to the state.
			// Note that there is no need to go back before largeStepTime
			// because these samples are independent of the sample on largeStepTime.
			if (u[i].modify < largeStepTime)
			{
				u[i].modify = largeStepTime;
				u[i].value = rng->Next();
			}

			// Lazy evaluation of Mutate
			while (u[i].modify < time - 1)
			{
				u[i].value = Mutate(u[i].value);
				u[i].modify++;
			}

			// Save state
			prevSamples.emplace_back(i, u[i]);

			// Update the modified time and value
			u[i].value = Mutate(u[i].value);
			u[i].modify++;
		}
	}

	return u[i].value;
}

Math::Float PSSMLTPrimarySample::Mutate( const Math::Float& value )
{
	Math::Float u = rng->Next();
	bool positive = u < Math::Float(0.5);

	// Convert to [0, 1]
	u = positive ? u * Math::Float(2) : Math::Float(2) * (u - Math::Float(0.5)); 

	Math::Float dv = s2 * std::exp(logRatio * u);

	Math::Float result = value;
	if (positive)
	{
		result += dv;
		if (result > Math::Float(1)) result -= Math::Float(1);
	}
	else
	{
		result -= dv;
		if (result < Math::Float(0)) result += Math::Float(1);
	}

	return result;
}

LM_PUBLIC_API void PSSMLTPrimarySample::SetLargeStep( bool largeStep )
{
	this->largeStep = largeStep;
}

LM_PUBLIC_API bool PSSMLTPrimarySample::LargeStep()
{
	return largeStep;
}

LM_PUBLIC_API void PSSMLTPrimarySample::SetRng( Random* rng )
{
	this->rng = rng;
}

LM_PUBLIC_API void PSSMLTPrimarySample::GetCurrentSampleState( std::vector<Math::Float>& samples ) const
{
	samples.clear();
	for (auto& sample : u)
	{
		samples.push_back(sample.value);
	}
}

LM_NAMESPACE_END