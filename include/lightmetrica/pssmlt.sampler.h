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
#ifndef LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H
#define LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H

#include "common.h"
#include "math.types.h"
#include "random.h"
#include <memory>
#include <vector>
#include <tuple>

LM_NAMESPACE_BEGIN

/*!
	PSSMLT sampler.
	Sampler interface for uniform random number generator
	used in the PSSMLT implementation.
*/
class PSSMLTSampler
{
public:

	PSSMLTSampler() {}
	virtual ~PSSMLTSampler() {}

private:

	LM_DISABLE_COPY_AND_MOVE(PSSMLTSampler);

public:
	
	LM_PUBLIC_API virtual Math::Float Next() = 0;
	LM_PUBLIC_API virtual Random* Rng() = 0;
	Math::Vec2 NextVec2() { return Math::Vec2(Next(), Next()); }

};

// --------------------------------------------------------------------------------

/*!
	Restorable sampler.
	An implementation of uniform random number generator
	which can be restored in the specified index.
*/
class PSSMLTRestorableSampler : public Sampler
{
public:

	LM_PUBLIC_API PSSMLTRestorableSampler(Random* rng, unsigned int seed);
	LM_PUBLIC_API PSSMLTRestorableSampler(Random* rng, const PSSMLTRestorableSampler& sampler);

	

};

/*
class PSSMLTRestorableSampler : public PSSMLTSampler
{
public:

	LM_PUBLIC_API PSSMLTRestorableSampler(Random* rng, unsigned int seed);
	LM_PUBLIC_API PSSMLTRestorableSampler(Random* rng, const PSSMLTRestorableSampler& sampler);

public:

	LM_PUBLIC_API Math::Float Next();
	LM_PUBLIC_API Random* Rng();

public:

	LM_PUBLIC_API void SetIndex(int index);
	LM_PUBLIC_API int Index();

private:

	unsigned int initialSeed;		//!< Initial seed
	std::unique_ptr<Random> rng;	//!< Random number generator
	int currentIndex;				//!< Number of generated samples

};
*/

// --------------------------------------------------------------------------------

/*!
	Kelemen's primary sample set.
	A element of the primary sample space.
	The class works as an implementation of the random number sampler
	and is usable in the renderer implementation without modification.
	For details, see the original paper.
*/
class PSSMLTPrimarySample : public PSSMLTSampler
{
public:

	struct Sample
	{
		Sample(const Math::Float& value)
			: value(value)
			, modify(0)
		{}

		Math::Float value;		//!< Sample value
		long long modify;		//!< Last modified time
	};

public:

	LM_PUBLIC_API PSSMLTPrimarySample(const Math::Float& s1, const Math::Float& s2);

public:

	LM_PUBLIC_API Math::Float Next();
	LM_PUBLIC_API Random* Rng();

public:

	LM_PUBLIC_API void Accept();
	LM_PUBLIC_API void Reject();

	LM_PUBLIC_API void SetLargeStep(bool largeStep);
	LM_PUBLIC_API bool LargeStep();
	LM_PUBLIC_API void SetRng(Random* rng);
	LM_PUBLIC_API void GetCurrentSampleState(std::vector<Math::Float>& samples) const;
	LM_PUBLIC_API void GetCurrentSampleState(std::vector<Math::Float>& samples, int numSamples);

	LM_PUBLIC_API void SetKernelSizeScale(const Math::Float& scale);
	
private:

	Math::Float PrimarySample(int i);
	Math::Float Mutate(const Math::Float& value);

private:

	Math::Float s1, s2;									//!< Kernel size parameters
	Math::Float logRatio;								//!< Temporary variable (for efficiency)

	Random* rng;										//!< Random number generator (not managed here)

	long long time;										//!< Number of accepted mutations
	long long largeStepTime;							//!< Time of the last accepted large step
	bool largeStep;										//!< Indicates the next mutation is the large step

	int currentIndex;									//!< Current sample index
	std::vector<Sample> u;								//!< List of current samples
	std::vector<std::tuple<int, Sample>> prevSamples;	//!< Temporary list for preserving previous samples (restored if rejected)

	Math::Float kernelSizeScale;						// TODO

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H