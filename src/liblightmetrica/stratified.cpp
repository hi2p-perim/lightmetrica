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
#include <lightmetrica/camerasampler.h>

LM_NAMESPACE_BEGIN

/*!
	Stratified sampler.
	Implements stratified sampling for samples for cameras.
*/
class StratifiedSampler final : public CameraSampler
{
public:

	LM_COMPONENT_IMPL_DEF("stratified");

public:

	virtual void GenerateSamples(Math::Vec2i& pixelPos) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual bool Configure(const ConfigNode& node, const Assets& assets) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Sampler* Clone() const override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual void SetSeed(unsigned int seed) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Math::Float Next() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual unsigned int NextUInt() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Math::Vec2 NextVec2() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual Random* Rng() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

};

LM_COMPONENT_REGISTER_IMPL(StratifiedSampler, ConfigurableSampler);

LM_NAMESPACE_END