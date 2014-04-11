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
#include <lightmetrica/expt.recordrmse.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/bitmaptexture.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/hdrfilm.h>
#include <lightmetrica/bitmap.h>

LM_NAMESPACE_BEGIN

class RecordRMSEExperiment::Impl
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
	BitmapTexture* referenceTexture;

private:

	HDRBitmapFilm* film;
	long long sample;
	Math::Float rmse;

private:

	std::vector<std::tuple<long long, Math::Float>> rmses;

};

bool RecordRMSEExperiment::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("frequency", 100LL, frequency);
	node.ChildValueOrDefault("output_path", std::string("rmse.txt"), outputPath);

	// Reference image
	auto referenceImageNode = node.Child("reference_image");
	if (referenceImageNode.Empty())
	{
		LM_LOG_ERROR("'reference_image' is required");
		return false;
	}

	// Resolve reference
	referenceTexture = dynamic_cast<BitmapTexture*>(assets.ResolveReferenceToAsset(referenceImageNode, "texture"));
	if (referenceTexture == nullptr)
	{
		return false;
	}

	return true;
}

void RecordRMSEExperiment::Impl::Notify( const std::string& type )
{
	if (type == "RenderStarted") HandleNotify_RenderStarted();
	else if (type == "SampleFinished") HandleNotify_SampleFinished();
	else if (type == "RenderFinished") HandleNotify_RenderFinished();
}

void RecordRMSEExperiment::Impl::UpdateParam( const std::string& name, const void* param )
{
	if (name == "film") film = (HDRBitmapFilm*)param;
	else if (name == "sample") sample = *(int*)param;
	else if (name == "rmse") rmse = *(Math::Float*)param;
}

void RecordRMSEExperiment::Impl::HandleNotify_RenderStarted()
{
	rmses.clear();
}

void RecordRMSEExperiment::Impl::HandleNotify_SampleFinished()
{
	if (sample % frequency == 0)
	{
		// Compute RMSE of current sample
		auto rmse = referenceTexture->Bitmap().EvaluateRMSE(
			film->Bitmap(),
			sample > 0
				? Math::Float(film->Width() * film->Height()) / Math::Float(sample)
				: Math::Float(1));
		rmses.emplace_back(sample, rmse);
	}
}

void RecordRMSEExperiment::Impl::HandleNotify_RenderFinished()
{	
	// Save RMSE plot
	LM_LOG_INFO("Saving RMSE plot to " + outputPath);
	std::ofstream ofs(outputPath);
	for (auto& v : rmses)
	{
		ofs << std::get<0>(v) << " " << std::get<1>(v) << std::endl;
	}
}

// --------------------------------------------------------------------------------

RecordRMSEExperiment::RecordRMSEExperiment()
	: p(new Impl)
{

}

RecordRMSEExperiment::~RecordRMSEExperiment()
{
	LM_SAFE_DELETE(p);
}

bool RecordRMSEExperiment::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

void RecordRMSEExperiment::Notify( const std::string& type )
{
	p->Notify(type);
}

void RecordRMSEExperiment::UpdateParam( const std::string& name, const void* param )
{
	p->UpdateParam(name, param);
}

LM_NAMESPACE_END