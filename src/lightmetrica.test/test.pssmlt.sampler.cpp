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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/rewindablesampler.h>
#include <lightmetrica/random.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class RewindableSamplerTest : public TestBase {};

TEST_F(RewindableSamplerTest, GenerateAndRestore)
{
	// Initialize using seed 1
	std::unique_ptr<RewindableSampler> sampler(ComponentFactory::Create<RewindableSampler>());
	sampler->Configure(ComponentFactory::Create<Random>("standardmt"));
	sampler->SetSeed(1);

	// Generate some samples
	const int Count = 1<<9;
	std::vector<Math::Float> samples;
	for (int i = 0; i < Count; i++)
	{
		samples.push_back(sampler->Next());
	}

	// Restore state and re-generate samples
	for (int index = 0; index < Count-1; index++)
	{
		sampler->Rewind(index);
		for (int i = index; i < Count; i++)
		{
			// Check generated values
			EXPECT_TRUE(ExpectNear(sampler->Next(), samples[i]));
		}
	}
}

// --------------------------------------------------------------------------------

class PSSMLTPrimarySampleTest : public TestBase
{
public:

	PSSMLTPrimarySampleTest()
		: Count(1<<9)
		, primarySample(ComponentFactory::Create<PSSMLTPrimarySampler>())
	{
		primarySample->Configure(
			ComponentFactory::Create<Random>("standardmt"),
			Math::Float(1) / Math::Float(1024),
			Math::Float(1) / Math::Float(64));
		primarySample->SetSeed(1);
	}

protected:

	int Count;
	std::unique_ptr<PSSMLTPrimarySampler> primarySample;

};

TEST_F(PSSMLTPrimarySampleTest, Reject)
{
	// Generate initial samples
	std::vector<Math::Float> samples;
	for (int i = 0; i < Count; i++)
	{
		samples.push_back(primarySample->Next());
	}
	primarySample->Accept();
	
	for (int mode = 0; mode < 2; mode++)
	{
		// Mutate samples by large step mutation or small step mutation
		primarySample->EnableLargeStepMutation(mode == 0);
		for (int i = 0; i < Count; i++)
		{
			primarySample->Next();
		}

		// Reject -> the sample should be the previous state
		primarySample->Reject();

		std::vector<Math::Float> currentSamples;
		primarySample->GetCurrentSampleState(currentSamples);
		for (int i = 0; i < Count; i++)
		{
			EXPECT_TRUE(ExpectNear(currentSamples[i], samples[i]));
		}
	}
}

TEST_F(PSSMLTPrimarySampleTest, Accept)
{
	// Generate initial samples
	for (int i = 0; i < Count; i++)
	{
		primarySample->Next();
	}
	primarySample->Accept();

	for (int mode = 0; mode < 2; mode++)
	{
		// Mutate samples by large step mutation or small step mutation
		primarySample->EnableLargeStepMutation(mode == 0);
		std::vector<Math::Float> samples;
		for (int i = 0; i < Count; i++)
		{
			samples.push_back(primarySample->Next());
		}

		// Reject -> the mutated sample is preserved
		primarySample->Accept();

		std::vector<Math::Float> currentSamples;
		primarySample->GetCurrentSampleState(currentSamples);
		for (int i = 0; i < Count; i++)
		{
			EXPECT_TRUE(ExpectNear(currentSamples[i], samples[i]));
		}
	}
}

TEST_F(PSSMLTPrimarySampleTest, Sequence)
{
	// Another random number generator
	std::unique_ptr<Random> rng(ComponentFactory::Create<Random>("standardmt"));

	// Generate initial samples
	const int Delta = 10;
	for (int i = 0; i < Delta; i++)
	{
		primarySample->Next();
	}
	primarySample->Accept();

	// Iterate sequence of events
	const int Iter = 1<<5;
	std::vector<Math::Float> current, proposed, state;
	for (int i = 0; i < Iter; i++)
	{
		// Current state
		primarySample->GetCurrentSampleState(current);

		// Large step or small step
		primarySample->EnableLargeStepMutation(rng->Next() < Math::Float(0.5));
		
		// Generate samples
		for (int j = 0; j < Delta; j++)
		{
			primarySample->Next();
		}

		// Proposed state
		primarySample->GetCurrentSampleState(proposed);

		// Accept or reject
		if (rng->Next() < Math::Float(0.5))
		{
			primarySample->Accept();
			
			// State must be #proposed
			primarySample->GetCurrentSampleState(state);
			EXPECT_TRUE(proposed.size() <= state.size());
			for (size_t k = 0; k < proposed.size(); k++)
			{
				EXPECT_TRUE(ExpectNear(state[k], proposed[k]));
			}
		}
		else
		{
			primarySample->Reject();

			// State must be #current
			primarySample->GetCurrentSampleState(state);
			EXPECT_TRUE(current.size() <= state.size());
			for (size_t k = 0; k < current.size(); k++)
			{
				EXPECT_TRUE(ExpectNear(state[k], current[k]));
			}
		}
	}
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
