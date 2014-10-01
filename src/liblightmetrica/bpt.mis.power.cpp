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
	Power heuristics MIS weight.
	Implements power heuristics.
*/
class BPTPowerHeuristicsMISWeight final : public BPTMISWeight
{
public:

	LM_COMPONENT_IMPL_DEF("power");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets) override;
	virtual BPTMISWeight* Clone() const override;
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const override;


private:

	// Beta coefficient for power heuristics
	Math::Float betaCoeff;

};

bool BPTPowerHeuristicsMISWeight::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("beta_coeff", Math::Float(2), betaCoeff);
	return true;
}

BPTMISWeight* BPTPowerHeuristicsMISWeight::Clone() const
{
	auto inst = new BPTPowerHeuristicsMISWeight;
	inst->betaCoeff = betaCoeff;
	return inst;
}

Math::Float BPTPowerHeuristicsMISWeight::Evaluate(const BPTFullPath& fullPath) const
{
	const int n = fullPath.s + fullPath.t;

	Math::Float ps = fullPath.EvaluateFullpathPDF(fullPath.s);
	if (Math::IsZero(ps))
	{
		return Math::Float(0);
	}

	// Inverse of the weight 1/w_{s,t}. Initial weight is p_s/p_s = 1
	Math::Float invWeight(1);

	// Iteratively compute p_i/p_s where i = s-1 downto 0
	Math::Float piDivPs(1);
	bool prevPDFIsZero = false;
	for (int i = fullPath.s-1; i >= 0; i--)
	{
		if (fullPath.FullpathPDFIsZero(i))
		{
			prevPDFIsZero = true;
			continue;
		}

		if (prevPDFIsZero)
		{
			piDivPs = fullPath.EvaluateFullpathPDF(i) / ps;
			prevPDFIsZero = false;
		}
		else
		{
			auto ratio = fullPath.EvaluateFullpathPDFRatio(i);
			if (Math::IsZero(ratio))
			{
				break;
			}

			piDivPs *= Math::Float(1) / ratio;
		}

		invWeight += piDivPs * piDivPs;
	}

	// Iteratively compute p_i/p_s where i = s+1 to n
	piDivPs = Math::Float(1);
	prevPDFIsZero = false;
	for (int i = fullPath.s; i < n; i++)
	{
		if (fullPath.FullpathPDFIsZero(i+1))
		{
			prevPDFIsZero = true;
			continue;
		}

		if (prevPDFIsZero)
		{
			piDivPs = fullPath.EvaluateFullpathPDF(i+1) / ps;
			prevPDFIsZero = false;
		}
		else
		{
			auto ratio = fullPath.EvaluateFullpathPDFRatio(i);
			if (Math::IsZero(ratio))
			{
				break;
			}

			piDivPs *= ratio;
		}

		invWeight += piDivPs * piDivPs;
	}

	return Math::Float(1) / invWeight;
}

LM_COMPONENT_REGISTER_IMPL(BPTPowerHeuristicsMISWeight, BPTMISWeight);

LM_NAMESPACE_END