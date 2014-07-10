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
	PSSMLT length.
	Traces the lengths of light paths.
*/
class PSSMLTLengthExperiment : public Experiment
{
public:

	LM_COMPONENT_IMPL_DEF("pssmltlength");

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

private:

	long long sample;
	int length;

private:

	std::vector<long long> sampleIndices;
	std::vector<int> records;

};

bool PSSMLTLengthExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("pssmltlength.txt"), outputPath);
	return true;
}

void PSSMLTLengthExperiment::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void PSSMLTLengthExperiment::UpdateParam( const std::string& name, const void* param )
{
	if (name == "sample") sample = *(int*)param;
	else if (name == "pssmlt_path_length") length = *(int*)param;
}

void PSSMLTLengthExperiment::HandleNotify_RenderStarted()
{
	sampleIndices.clear();
	records.clear();
}

void PSSMLTLengthExperiment::HandleNotify_SampleFinished()
{
	if (sample % frequency == 0)
	{
		// Records sample
		sampleIndices.push_back(sample);
		records.push_back(length);
	}
}

void PSSMLTLengthExperiment::HandleNotify_RenderFinished()
{
	// Save records
	LM_LOG_INFO("Saving PSSMLT path length to " + outputPath);
	LM_LOG_INDENTER();

	std::ofstream ofs(outputPath);
	for (size_t i = 0; i < sampleIndices.size(); i++)
	{
		ofs << sampleIndices[i] << " " << records[i] << std::endl;
	}

	LM_LOG_INFO("Successfully saved " + std::to_string(sampleIndices.size()) + " entries");
}

LM_COMPONENT_REGISTER_IMPL(PSSMLTLengthExperiment, Experiment);

LM_NAMESPACE_END