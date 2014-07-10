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
#ifndef LIB_LIGHTMETRICA_EXPERIMENT_H
#define LIB_LIGHTMETRICA_EXPERIMENT_H

#include "component.h"
#include <string>

LM_NAMESPACE_BEGIN

class ConfigNode;
class Assets;

/*!
	Experiment.
	The base class of experiments for renderers.
*/
class Experiment : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("experiments")

public:

	Experiment() {}
	virtual ~Experiment() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Experiment);

public:

	/*!
		Configure the experiment.
		\param node A configuration node which consists of \a experiment element.
		\retval true Succeeded to configure.
		\retval false Failed to configure. 
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets) = 0;

	/*!
		Notify an event.
		\param type Event type.
	*/
	virtual void Notify(const std::string& type) = 0;

	/*!
		Update parameter.
		\param name Parameter name.
		\param param Parameter.
	*/
	virtual void UpdateParam(const std::string& name, const void* param) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_EXPERIMENT_H