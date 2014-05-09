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
#ifndef LIB_LIGHTMETRICA_DEFAULT_EXPTS_H
#define LIB_LIGHTMETRICA_DEFAULT_EXPTS_H

#include "expts.h"
#include <vector>

LM_NAMESPACE_BEGIN

class Experiment;

/*!
	Default experiments.
	Default implementation for Experiments class.
*/
class LM_PUBLIC_API DefaultExperiments : public Experiments
{
public:

	DefaultExperiments();
	~DefaultExperiments();

public:

	virtual bool Configure( const ConfigNode& node, const Assets& assets );
	virtual void Notify( const std::string& type );
	virtual void UpdateParam( const std::string& name, const void* param );
	virtual bool CheckConfigured();

public:

	/*!
		Load experiments.
		Load experiment instances.
		This function is used internally for testing.
		The ownership of the given pointers is delegated to the experiments manager.
		We note that it is valid because #Experiment is inherited from #Object
		and any instances is allocated in the dynamic library side with overloaded operator new.
		\param experiments List of experiments.
	*/
	bool LoadExperiments(const std::vector<Experiment*>& experiments);

	/*!
		Get experiment by name.
		\param name Experiment name.
		\return Experiment.
	*/
	const Experiment* ExperimentByName(const std::string& name) const;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DEFAULT_EXPTS_H