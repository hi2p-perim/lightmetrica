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
