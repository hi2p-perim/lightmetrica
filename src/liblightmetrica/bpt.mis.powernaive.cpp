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
	Math::Float invWeight(1);
	Math::Float ps = fullPath.EvaluateFullpathPDF(fullPath.s);
	if (Math::IsZero(ps))
	{
		// p_s might be zero because of special handling of geometry term.
		// In this case, the full-path is samples have some contribution value, but zero probability.
		return Math::Float(0);
	}

	for (int i = 0; i <= fullPath.s + fullPath.t; i++)
	{
		if (i == fullPath.s)
		{
			// Note that due to specular connections,
			// full-path PDF evaluation for p_s might not be correct, so
			// we precomputed the value of p_s / p_s = 1 as an initial value of inverse weight.
			// Otherwise, unnatural black points is observed in the rendered image.
			continue;
		}

		auto pi = fullPath.EvaluateFullpathPDF(i);
		if (Math::IsZero(pi))
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