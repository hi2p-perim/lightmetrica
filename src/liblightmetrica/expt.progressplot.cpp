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

LM_NAMESPACE_BEGIN

/*!
	Progress plot.
	Experiment on tracing progress plot w.r.t. time.
*/
class ProgressPlotExperiment : public Experiment
{
public:

	LM_COMPONENT_IMPL_DEF("progressplot");

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void Notify( const std::string& type );
	virtual void UpdateParam( const std::string& name, const void* param );

public:

	void HandleNotify_RenderStarted();
	void HandleNotify_ProgressUpdated();
	void HandleNotify_RenderFinished();

public:

	long long frequency;
	std::string outputPath;

public:

	long long block;
	double progress;

public:

	// Time - progress
	std::vector<std::tuple<long long, double>> records;
	std::chrono::high_resolution_clock::time_point start;

};

bool ProgressPlotExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("progress.txt"), outputPath);
	return true;
}

void ProgressPlotExperiment::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "ProgressUpdated") HandleNotify_ProgressUpdated();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void ProgressPlotExperiment::UpdateParam( const std::string& name, const void* param )
{
	if (name == "block") block = *(int*)param;
	else if (name == "progress") progress = *(double*)param;
}

void ProgressPlotExperiment::HandleNotify_RenderStarted()
{
	records.clear();
}

void ProgressPlotExperiment::HandleNotify_ProgressUpdated()
{
	if (block == 0)
	{
		start = std::chrono::high_resolution_clock::now();
	}

	if (block % frequency == 0)
	{
		auto now = std::chrono::high_resolution_clock::now();
		long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
		records.emplace_back(elapsed, progress);
	}
}

void ProgressPlotExperiment::HandleNotify_RenderFinished()
{
	// Save progress plot
	LM_LOG_INFO("Saving progress plot to " + outputPath);
	LM_LOG_INDENTER();

	std::ofstream ofs(outputPath);
	for (auto& v : records)
	{
		ofs << std::get<0>(v) << " " << std::get<1>(v) << std::endl;
	}

	LM_LOG_INFO("Successfully saved " + std::to_string(records.size()) + " entries");
}

LM_COMPONENT_REGISTER_IMPL(ProgressPlotExperiment, Experiment);

LM_NAMESPACE_END
