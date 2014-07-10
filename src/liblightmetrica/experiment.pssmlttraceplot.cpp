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
#include <lightmetrica/experiment.h>
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