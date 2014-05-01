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

public:

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
		Evaluate MIS weight w_{s,t}.
		\param fullPath Full-path.
		\return MIS weight.
	*/
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_MIS_H