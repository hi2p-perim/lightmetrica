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
#ifndef LIB_LIGHTMETRICA_CONFIGUABLE_SAMPLER_H
#define LIB_LIGHTMETRICA_CONFIGUABLE_SAMPLER_H

#include "sampler.h"

LM_NAMESPACE_BEGIN

/*!
	Configurable sampler.
	An interface for sampler classes which are able to configure with a config node.
*/
class ConfigurableSampler : public Sampler
{
public:

	LM_COMPONENT_INTERFACE_DEF("configurablesampler");

public:

	ConfigurableSampler() {}
	virtual ~ConfigurableSampler() {}

public:

	/*!
		Configure.
		Configure and initialize the sampler by the XML elements given by #node.
		\param node XML node for the configuration.
		\param assets Asset manager.
		\param true Succeeded to configure.
		\param false Failed to configure.
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_CONFIGUABLE_SAMPLER_H