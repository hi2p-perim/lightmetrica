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

#include "pch.h"
#include <lightmetrica/bpt.mis.h>
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/renderutils.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

/*!
	Power heuristics MIS weight (naive version).
	Implements power heuristics.
*/
class BPTPowerHeuristicsNaiveMISWeight : public BPTMISWeight
{
public:

	LM_COMPONENT_IMPL_DEF("powernaive");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets);
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const;

private:

	// Beta coefficient for power heuristics
	Math::Float betaCoeff;

};

bool BPTPowerHeuristicsNaiveMISWeight::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("beta_coeff", Math::Float(2), betaCoeff);
	return true;
}

Math::Float BPTPowerHeuristicsNaiveMISWeight::Evaluate(const BPTFullPath& fullPath) const
{
	Math::Float invWeight(0);
	Math::Float ps = fullPath.EvaluateFullpathPDF(fullPath.s);
	LM_ASSERT(Math::Abs(ps) > Math::Constants::Eps());

	for (int i = 0; i <= fullPath.s + fullPath.t; i++)
	{
		auto pi = fullPath.EvaluateFullpathPDF(i);
		//if (Math::IsZero(pi))
		//{
		//	continue;
		//}
		if (Math::Abs(pi) < Math::Constants::Eps())
		{
			continue;
		}

		auto ratio = pi / ps;
		invWeight += ratio * ratio;
	}

	return Math::Float(1) / invWeight;
}

LM_COMPONENT_REGISTER_IMPL(BPTPowerHeuristicsNaiveMISWeight, BPTMISWeight);

LM_NAMESPACE_END