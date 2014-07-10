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
