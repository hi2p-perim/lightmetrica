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
#include <lightmetrica/expt.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/assert.h>

LM_NAMESPACE_BEGIN

#if 0

/*!
	PSSMLT running mean.
	Traces running means of primary samples.
	Running mean is a plot of the iteration against
	the mean of the draws up to the iteration.
	Running mean plot is used for the evaluation of the mixing.
*/
class PSSMLTRunningMeanExperiment : public Experiment
{
public:

	LM_COMPONENT_IMPL_DEF("pssmltrunningmean");

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void Notify( const std::string& type );
	virtual void UpdateParam( const std::string& name, const void* param );

private:

	void HandleNotify_RenderStarted();
	void HandleNotify_SampleFinished();
	void HandleNotify_RenderFinished();

private:

	long long frequency;
	std::string outputPath;
	int traceNumSamples;

private:

	long long sample;
	PSSMLTPrimarySampler* primarySample;

private:

	std::vector<Math::Float> sampleValueSums;
	std::vector<long long> sampleIndices;
	std::vector<std::vector<Math::Float>> records;

};

bool PSSMLTRunningMeanExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("pssmltrunningmean.txt"), outputPath);
	node.ChildValueOrDefault("trace_num_samples", 1, traceNumSamples);
	return true;
}

void PSSMLTRunningMeanExperiment::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void PSSMLTRunningMeanExperiment::UpdateParam( const std::string& name, const void* param )
{
	if (name == "sample") sample = *(int*)param;
	else if (name == "pssmlt_primary_sample") primarySample = (PSSMLTPrimarySampler*)param;
}

void PSSMLTRunningMeanExperiment::HandleNotify_RenderStarted()
{
	sampleValueSums.assign(traceNumSamples, Math::Float(0));
	sampleIndices.clear();
	records.clear();
}

void PSSMLTRunningMeanExperiment::HandleNotify_SampleFinished()
{
	std::vector<Math::Float> currentSamples;
	primarySample->GetCurrentSampleState(currentSamples, traceNumSamples);

	LM_ASSERT(sampleValueSums.size() == currentSamples.size());
	for (size_t i = 0; i < sampleValueSums.size(); i++)
	{
		sampleValueSums[i] += currentSamples[i];
	}

	if (sample % frequency == 0 && sample > 0)
	{
		auto tmp = sampleValueSums;
		for (auto& v : tmp)
		{
			v /= Math::Float(sample);
		}

		sampleIndices.push_back(sample);
		records.emplace_back(std::move(tmp));
	}
}

void PSSMLTRunningMeanExperiment::HandleNotify_RenderFinished()
{
	// Save records
	LM_LOG_INFO("Saving PSSMLT running mean plot to " + outputPath);
	LM_LOG_INDENTER();

	std::ofstream ofs(outputPath);
	for (size_t i = 0; i < sampleIndices.size(); i++)
	{
		ofs << sampleIndices[i] << " ";
		auto& row = records[i];
		for (auto& v : row)
		{
			ofs << v << " ";
		}
		ofs << std::endl;
	}

	LM_LOG_INFO("Successfully saved " + std::to_string(sampleIndices.size()) + " entries");
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTRunningMeanExperiment, Experiment);

#endif

LM_NAMESPACE_END