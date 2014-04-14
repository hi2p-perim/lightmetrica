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
#include <lightmetrica/expt.pssmltacceptanceratio.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>

LM_NAMESPACE_BEGIN

class PSSMLTAcceptanceRatioExperiment::Impl
{
public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	void Notify( const std::string& type );
	void UpdateParam( const std::string& name, const void* param );

private:

	void HandleNotify_RenderStarted();
	void HandleNotify_SampleFinished();
	void HandleNotify_RenderFinished();

private:

	long long frequency;
	std::string outputPath;

private:

	long long sample;
	Math::Float acceptanceRatio;

private:

	std::vector<long long> sampleIndices;
	std::vector<Math::Float> records;

};

bool PSSMLTAcceptanceRatioExperiment::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("pssmlttraceplot.txt"), outputPath);
	return true;
}

void PSSMLTAcceptanceRatioExperiment::Impl::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void PSSMLTAcceptanceRatioExperiment::Impl::UpdateParam( const std::string& name, const void* param )
{
	if (name == "sample") sample = *(int*)param;
	else if (name == "pssmlt_acceptance_ratio") acceptanceRatio = *(Math::Float*)param;
}

void PSSMLTAcceptanceRatioExperiment::Impl::HandleNotify_RenderStarted()
{
	sampleIndices.clear();
	records.clear();
}

void PSSMLTAcceptanceRatioExperiment::Impl::HandleNotify_SampleFinished()
{
	if (sample % frequency == 0)
	{
		sampleIndices.push_back(sample);
		records.push_back(acceptanceRatio);
	}
}

void PSSMLTAcceptanceRatioExperiment::Impl::HandleNotify_RenderFinished()
{
	// Save records
	LM_LOG_INFO("Saving PSSMLT acceptance ratio to " + outputPath);
	LM_LOG_INDENTER();

	std::ofstream ofs(outputPath);
	for (size_t i = 0; i < sampleIndices.size(); i++)
	{
		ofs << sampleIndices[i] << " " << records[i] << std::endl;
	}

	LM_LOG_INFO("Successfully saved " + std::to_string(sampleIndices.size()) + " entries");
}

// --------------------------------------------------------------------------------

PSSMLTAcceptanceRatioExperiment::PSSMLTAcceptanceRatioExperiment()
	: p(new Impl)
{

}

PSSMLTAcceptanceRatioExperiment::~PSSMLTAcceptanceRatioExperiment()
{
	LM_SAFE_DELETE(p);
}

bool PSSMLTAcceptanceRatioExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

void PSSMLTAcceptanceRatioExperiment::Notify( const std::string& type )
{
	p->Notify(type);
}

void PSSMLTAcceptanceRatioExperiment::UpdateParam( const std::string& name, const void* param )
{
	p->UpdateParam(name, param);
}

LM_NAMESPACE_END