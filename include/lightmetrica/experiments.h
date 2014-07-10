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