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
#include <lightmetrica/logger.h>
#include <lightmetrica/bitmapfilm.h>

LM_NAMESPACE_BEGIN

/*!
	Record per sample images.
	An experiment for recording images per samples / mutations.
*/
class RecordImageExperiment : public Experiment
{
public:

	LM_COMPONENT_IMPL_DEF("recordimage");

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void Notify( const std::string& type );
	virtual void UpdateParam( const std::string& name, const void* param );

private:

	void HandleNotify_RenderStarted();
	void HandleNotify_SampleFinished();

private:

	long long frequency;
	std::string outputDir;

private:

	BitmapFilm* film;
	long long sample;

};

bool RecordImageExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_dir", std::string("images"), outputDir);
	return true;
}

void RecordImageExperiment::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
}

void RecordImageExperiment::UpdateParam( const std::string& name, const void* param )
{
	if (name == "film") film = (BitmapFilm*)param;
	else if (name == "sample") sample = *(int*)param;
}

void RecordImageExperiment::HandleNotify_RenderStarted()
{
	// Create output directory if it does not exists
	if (!boost::filesystem::exists(outputDir))
	{
		LM_LOG_INFO("Creating directory : " + outputDir);
		if (!boost::filesystem::create_directory(outputDir))
		{
			LM_LOG_WARN("Failed to create output directory : " + outputDir);
		}
	}
}

void RecordImageExperiment::HandleNotify_SampleFinished()
{
	if (sample % frequency == 0)
	{
		// Save intermediate image
		auto path = boost::filesystem::path(outputDir) / boost::str(boost::format("%010d.hdr") % sample);
		LM_LOG_INFO("Saving " + path.string());
		LM_LOG_INDENTER();
		film->RescaleAndSave(
			path.string(),
			sample > 0
				? Math::Float(film->Width() * film->Height()) / Math::Float(sample)
				: Math::Float(1));
	}
}

LM_COMPONENT_REGISTER_IMPL(RecordImageExperiment, Experiment);

LM_NAMESPACE_END