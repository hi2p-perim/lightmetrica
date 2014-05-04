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
#include <lightmetrica/expt.recordimage.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/bitmapfilm.h>

LM_NAMESPACE_BEGIN

class RecordImageExperiment::Impl
{
public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	void Notify( const std::string& type );
	void UpdateParam( const std::string& name, const void* param );

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

bool RecordImageExperiment::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_dir", std::string("images"), outputDir);
	return true;
}

void RecordImageExperiment::Impl::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
}

void RecordImageExperiment::Impl::UpdateParam( const std::string& name, const void* param )
{
	if (name == "film") film = (BitmapFilm*)param;
	else if (name == "sample") sample = *(int*)param;
}

void RecordImageExperiment::Impl::HandleNotify_RenderStarted()
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

void RecordImageExperiment::Impl::HandleNotify_SampleFinished()
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

// --------------------------------------------------------------------------------

RecordImageExperiment::RecordImageExperiment()
	: p(new Impl)
{

}

RecordImageExperiment::~RecordImageExperiment()
{
	LM_SAFE_DELETE(p);
}

bool RecordImageExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

void RecordImageExperiment::Notify( const std::string& type )
{
	p->Notify(type);
}

void RecordImageExperiment::UpdateParam( const std::string& name, const void* param )
{
	p->UpdateParam(name, param);
}

LM_NAMESPACE_END