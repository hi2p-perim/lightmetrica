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

LM_NAMESPACE_BEGIN

#if 0

/*!
	PSSMLT traceplot.
	Traces sample plots through PSSMLT updates.
*/
class PSSMLTTraceplotExperiment : public Experiment
{
public:

	LM_COMPONENT_IMPL_DEF("pssmlttraceplot");

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

	std::vector<long long> sampleIndices;
	std::vector<std::vector<Math::Float>> records;

};

bool PSSMLTTraceplotExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("pssmlttraceplot.txt"), outputPath);
	node.ChildValueOrDefault("trace_num_samples", 1, traceNumSamples);
	return true;
}

void PSSMLTTraceplotExperiment::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void PSSMLTTraceplotExperiment::UpdateParam( const std::string& name, const void* param )
{
	if (name == "sample") sample = *(int*)param;
	else if (name == "pssmlt_primary_sample") primarySample = (PSSMLTPrimarySampler*)param;
}

void PSSMLTTraceplotExperiment::HandleNotify_RenderStarted()
{
	sampleIndices.clear();
	records.clear();
}

void PSSMLTTraceplotExperiment::HandleNotify_SampleFinished()
{
	if (sample % frequency == 0)
	{
		// Get current state
		std::vector<Math::Float> currentSamples;
		primarySample->GetCurrentSampleState(currentSamples, traceNumSamples);

		// Records sample
		sampleIndices.push_back(sample);
		records.emplace_back(std::move(currentSamples));
	}
}

void PSSMLTTraceplotExperiment::HandleNotify_RenderFinished()
{
	// Save records
	LM_LOG_INFO("Saving PSSMLT traceplot to " + outputPath);
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

LM_COMPONENT_REGISTER_IMPL(PSSMLTTraceplotExperiment, Experiment);

#endif

LM_NAMESPACE_END