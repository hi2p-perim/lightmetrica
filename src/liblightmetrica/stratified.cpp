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
#include <lightmetrica/camerasampler.h>

LM_NAMESPACE_BEGIN

/*!
	Stratified sampler.
	Implements stratified sampling for samples for cameras.
*/
class StratifiedSampler : public CameraSampler
{
public:

	LM_COMPONENT_IMPL_DEF("stratified");

public:

	virtual void GenerateSamples( Math::Vec2i& pixelPos )
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual bool Configure( const ConfigNode& node, const Assets& assets )
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Sampler* Clone()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual void SetSeed( unsigned int seed )
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Math::Float Next()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual unsigned int NextUInt()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Math::Vec2 NextVec2()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

};

LM_COMPONENT_REGISTER_IMPL(StratifiedSampler, ConfiguableSampler);

LM_NAMESPACE_END