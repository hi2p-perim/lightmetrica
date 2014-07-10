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
#ifndef LIB_LIGHTMETRICA_BPT_MIS_H
#define LIB_LIGHTMETRICA_BPT_MIS_H

#include "bpt.common.h"
#include "component.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class BPTFullPath;
class ConfigNode;
class Assets;

/*!
	MIS weighting function for Veach's BPT.
	Veach's BPT requires to compute weighting function for full-path.
	Various techniques can be considered so we separated the implmenetations
	as component classes.
*/
class BPTMISWeight : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("bpt.mis");

public:

	BPTMISWeight() {}
	virtual ~BPTMISWeight() {}

private:

	LM_DISABLE_COPY_AND_MOVE(BPTMISWeight);

public:

	/*!
		Configure.
		Configures MIS weighting function.
		\param node A XML element which consists of \a mis_weight element.
		\param assets Assets manager.
		\retval true Succeeded to configure.
		\retval false Failed to configure.
	*/
	virtual bool Configure(const ConfigNode& node, const Assets& assets) = 0;
	
	/*!
		Clone the instance.
		\return Duplicated instance.
	*/
	virtual BPTMISWeight* Clone() const = 0;

	/*!
		Evaluate MIS weight w_{s,t}.
		\param fullPath Full-path.
		\return MIS weight.
	*/
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_MIS_H