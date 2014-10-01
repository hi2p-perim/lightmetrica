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
class BPTPowerHeuristicsNaiveMISWeight final : public BPTMISWeight
{
public:

	LM_COMPONENT_IMPL_DEF("powernaive");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets) override;
	virtual BPTMISWeight* Clone() const override;
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const override;

private:

	// Beta coefficient for power heuristics
	Math::Float betaCoeff;

};

bool BPTPowerHeuristicsNaiveMISWeight::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("beta_coeff", Math::Float(2), betaCoeff);
	return true;
}

BPTMISWeight* BPTPowerHeuristicsNaiveMISWeight::Clone() const
{
	auto inst = new BPTPowerHeuristicsNaiveMISWeight;
	inst->betaCoeff = betaCoeff;
	return inst;
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
			// Otherwise, unnatural black points are observed in the rendered image.
			continue;
		}

		auto pi = fullPath.EvaluateFullpathPDF(i);
		if (pi > Math::Float(0))
		{
			auto ratio = pi / ps;
			invWeight += ratio * ratio;
		}
	}

	return Math::Float(1) / invWeight;
}

LM_COMPONENT_REGISTER_IMPL(BPTPowerHeuristicsNaiveMISWeight, BPTMISWeight);

LM_NAMESPACE_END