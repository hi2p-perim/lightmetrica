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