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

#pragma once
#ifndef LIB_LIGHTMETRICA_CAMERA_SAMPLER_H
#define LIB_LIGHTMETRICA_CAMERA_SAMPLER_H

#include "configurablesampler.h"

LM_NAMESPACE_BEGIN

/*!
	Camera sampler.
	A sampler interface for Camera.
	The class is limited to some renderer implementations
	based on the sampling of eye subpaths.
*/
class CameraSampler : public ConfigurableSampler
{
public:

	CameraSampler() {}
	virtual ~CameraSampler() {}

public:

	/*!
		Generate samples for a pixel.
		This function must be called before sampling process of a pixel.
		\param pixelPos Pixel position.
	*/
	virtual void GenerateSamples(Math::Vec2i& pixelPos) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_CAMERA_SAMPLER_H