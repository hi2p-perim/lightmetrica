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
#ifndef LIB_LIGHTMETRICA_DEFAULT_EXPTS_H
#define LIB_LIGHTMETRICA_DEFAULT_EXPTS_H

#include "experiments.h"
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