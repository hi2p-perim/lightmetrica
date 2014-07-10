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
#ifndef LIB_LIGHTMETRICA_EXPERIMENTS_H
#define LIB_LIGHTMETRICA_EXPERIMENTS_H

#include "common.h"
#include <string>

LM_NAMESPACE_BEGIN

class ConfigNode;
class Assets;

/*!
	Experiments.
	An interface for experiments manager classes.
	Experiments manager is responsible for managing experiments for renderers.
*/
class LM_PUBLIC_API Experiments
{
public:

	Experiments() {}
	virtual ~Experiments() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Experiments);

public:

	/*!
		Configure experiments from configuration node.
		\param node A configuration node which consists of \a experiments element.
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

	/*!
		Check if the experiment manager is configured.
		\retval true The experiment manager is configured.
		\retval false The experiment manager is not configured.
	*/
	virtual bool CheckConfigured() = 0;

};

LM_NAMESPACE_END

#if LM_EXPERIMENTAL_MODE
	
	#define LM_EXPT_NOTIFY(expts, type) \
		do { \
			if (expts.CheckConfigured()) \
				expts.Notify(type); \
		} while (0)

	#define LM_EXPT_UPDATE_PARAM(expts, name, param) \
		do { \
			if (expts.CheckConfigured()) \
				expts.UpdateParam(name, param); \
		} while (0)

#else

	#define LM_EXPT_NOTIFY(expts, type)
	#define LM_EXPT_UPDATE_PARAM(expts, name, param)

#endif

#endif // LIB_LIGHTMETRICA_EXPERIMENTS_H