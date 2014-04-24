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
	Power heuristics MIS weight.
	Implements power heuristics.
*/
class BPTPowerHeuristicsMISWeight : public BPTMISWeight
{
public:

	LM_COMPONENT_IMPL_DEF("bpt.mis.powerheuristics");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets);
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const;

private:

	// Evaluate p_{i+1}(\bar{x}_{s,t})/p_i(\bar{x}_{s,t}).
	Math::Float EvaluateSubsequentProbRatio(int i, const BPTFullPath& fullPath) const;

	// Get i-th vertex of the full-path
	const BPTPathVertex* FullPathVertex(int i, const BPTFullPath& fullPath) const;

	// Get i-th directional PDF evaluation of the full-path
	Math::PDFEval FullPathVertexDirectionPDF(int i, const BPTFullPath& fullPath, TransportDirection transportDir) const;

private:

	// Beta coefficient for power heuristics
	Math::Float betaCoeff;

};

bool BPTPowerHeuristicsMISWeight::Configure( const ConfigNode& node, const Assets& assets )
{
	node.ChildValueOrDefault("beta_coeff", Math::Float(2), betaCoeff);
	return true;
}

Math::Float BPTPowerHeuristicsMISWeight::Evaluate(const BPTFullPath& fullPath) const
{
	const int n = fullPath.s + fullPath.t;

	// Inverse of the weight 1/w_{s,t}. Initial weight is p_s/p_s = 1
	Math::Float invWeight(1);
	Math::Float piDivPs;

	// Iteratively compute p_i/p_s where i = s-1 downto 0
	piDivPs = Math::Float(1);
	for (int i = fullPath.s-1; i >= 0; i--)
	{
		Math::Float ratio = EvaluateSubsequentProbRatio(i, fullPath);
		if (Math::IsZero(ratio))
		{
			break;
		}
		piDivPs *= ratio;
		invWeight += piDivPs * piDivPs;
	}

	// Iteratively compute p_i/p_s where i = s+1 to n
	piDivPs = Math::Float(1);
	for (int i = fullPath.s+1; i < n; i++)
	{
		Math::Float ratio = EvaluateSubsequentProbRatio(i, fullPath);
		if (Math::IsZero(ratio))
		{
			break;
		}
		piDivPs *= Math::Float(1) / ratio;
		invWeight += piDivPs * piDivPs;
	}

	LM_LOG_DEBUG(std::to_string(invWeight));

	return Math::Float(1) / invWeight;
}

Math::Float BPTPowerHeuristicsMISWeight::EvaluateSubsequentProbRatio( int i, const BPTFullPath& fullPath ) const
{
	int n = fullPath.s + fullPath.t;
	if (i == 0)
	{
		// p_1 / p_0 =
		//    p_A(x_0) /
		//    p_{\sigma^\bot}(x_1\to x_0)G(x_1\leftrightarrow x_0)
		const auto* x0	= FullPathVertex(i, fullPath);
		const auto* x1	= FullPathVertex(i+1, fullPath);
		auto x1PdfDEL	= FullPathVertexDirectionPDF(i+1, fullPath, TransportDirection::EL);

		if (Math::IsZero(x0->pdfP.v))
		{
			return Math::Float(0);
		}
		else
		{
			LM_ASSERT(x0->pdfP.measure == Math::ProbabilityMeasure::Area);
			LM_ASSERT(x1PdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

			return
				x0->pdfP.v / x1PdfDEL.v /
				RenderUtils::GeneralizedGeometryTerm(x0->geom, x1->geom);
		}
	}
	else if (i == n-1)
	{
		// p_n / p_{n-1} =
		//     p_{\sigma^\bot}(x_{n-2}\to x_{n-1})G(x_{n-2}\leftrightarrow x_{n-1}) /
		//     p_A(x_{n-1})
		const auto* xn		= FullPathVertex(n-1, fullPath);
		const auto* xnPrev	= FullPathVertex(n-2, fullPath);
		auto xnPrevPdfDLE	= FullPathVertexDirectionPDF(n-2, fullPath, TransportDirection::LE);

		if (Math::IsZero(xn->pdfP.v))
		{
			return Math::Float(0);
		}
		else
		{
			LM_ASSERT(xn->pdfP.measure == Math::ProbabilityMeasure::Area);
			LM_ASSERT(xnPrevPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

			return
				xnPrevPdfDLE.v *
				RenderUtils::GeneralizedGeometryTerm(xnPrev->geom, xn->geom) /
				xn->pdfP.v;
		}
	}
	else
	{
		// p_{i+1} / p_i =
		//     p_{\sigma^\bot}(x_{i-1}\to x_i)G(x_{i-1}\leftrightarrow x_i) /
		//     p_{\sigma^\bot}(x_{i+1}\to x_i)G(x_{i+1}\leftrightarrow x_i)
		const auto* xi		= FullPathVertex(i, fullPath);
		const auto* xiNext	= FullPathVertex(i+1, fullPath);
		const auto* xiPrev	= FullPathVertex(i-1, fullPath);
		auto xiPrevPdfDLE	= FullPathVertexDirectionPDF(i-1, fullPath, TransportDirection::LE);
		auto xiNextPdfDEL	= FullPathVertexDirectionPDF(i+1, fullPath, TransportDirection::EL);

		LM_ASSERT(xiPrevPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
		LM_ASSERT(xiNextPdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

		return
			xiPrevPdfDLE.v *
			RenderUtils::GeneralizedGeometryTerm(xiPrev->geom, xi->geom) /
			xiNextPdfDEL.v /
			RenderUtils::GeneralizedGeometryTerm(xiNext->geom, xi->geom);
	}

	return Math::Float(0);
}

const BPTPathVertex* BPTPowerHeuristicsMISWeight::FullPathVertex( int i, const BPTFullPath& fullPath ) const
{
	LM_ASSERT(0 <= i && i < fullPath.s + fullPath.t);
	return
		i < fullPath.s
			? fullPath.lightSubpath.vertices[i]
			: fullPath.eyeSubpath.vertices[fullPath.t-1-(i-fullPath.s)];
}

Math::PDFEval BPTPowerHeuristicsMISWeight::FullPathVertexDirectionPDF( int i, const BPTFullPath& fullPath, TransportDirection transportDir ) const
{
	LM_ASSERT(0 <= i && i < fullPath.s + fullPath.t);
	return
		i == fullPath.s - 1	? fullPath.pdfDL[transportDir] :
		i == fullPath.s		? fullPath.pdfDE[transportDir]
							: FullPathVertex(i, fullPath)->pdfD[transportDir];
}

LM_COMPONENT_REGISTER_IMPL(BPTPowerHeuristicsMISWeight, BPTMISWeight);

LM_NAMESPACE_END