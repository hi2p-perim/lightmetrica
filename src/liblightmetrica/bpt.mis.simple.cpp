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
#include <lightmetrica/generalizedbsdf.h>

LM_NAMESPACE_BEGIN

/*!
	Simple MIS weight.
	Defines simple MIS weight.
	Simply reciprocal of # of full-paths with positive probability.
*/
class BPTSimpleMISWeight : public BPTMISWeight
{
public:

	LM_COMPONENT_IMPL_DEF("simple");

public:

	virtual bool Configure(const ConfigNode& node, const Assets& assets) { return true; }
	virtual BPTMISWeight* Clone() const { return new BPTSimpleMISWeight; }
	virtual Math::Float Evaluate(const BPTFullPath& fullPath) const;

};

Math::Float BPTSimpleMISWeight::Evaluate(const BPTFullPath& fullPath) const
{
#if 1

	int nonZeroProbPaths = 0;
	const int n = fullPath.s + fullPath.t;
	for (int i = 0; i <= n; i++)
	{
		if (!fullPath.FullpathPDFIsZero(i))
		{
			nonZeroProbPaths++;
		}
	}

	return Math::Float(1) / Math::Float(nonZeroProbPaths);

#elif 1

	int nonZeroProbPaths = 1;
	const int n = fullPath.s + fullPath.t;
	for (int i = 0; i <= n; i++)
	{
		if (i == fullPath.s)
		{
			// Already counted
			continue;
		}

		if (i == 0)
		{
			const auto* p0 = fullPath.FullPathVertex(0);
			if (p0->areaLight != nullptr && !p0->geom.degenerated)
			{
				nonZeroProbPaths++;
			}
		}
		else if (i == n)
		{
			const auto* pn = fullPath.FullPathVertex(n-1);
			if (pn->areaCamera != nullptr && !pn->geom.degenerated)
			{
				nonZeroProbPaths++;
			}
		}
		else if (0 < i && i < n)
		{
			// Check if one of connection vertices are degenerated
			// which cannot be sampled by p_i
			const auto* pi     = fullPath.FullPathVertex(i-1);
			const auto* piNext = fullPath.FullPathVertex(i);
			if (!pi->bsdf->Degenerated() && !piNext->bsdf->Degenerated())
			{
				nonZeroProbPaths++;
			}
		}
	}

	return Math::Float(1) / Math::Float(nonZeroProbPaths);

#elif 0

	int nonZeroProbPaths2 = 1;
	const int n = fullPath.s + fullPath.t;
	for (int i = 0; i <= n; i++)
	{
	     if (i == fullPath.s)
	     {
	          // Already counted
	          continue;
	     }
	    
	     auto pi = fullPath.EvaluateFullpathPDF(i);
	     if (pi > Math::Float(0))
	     {
	          nonZeroProbPaths2++;
	     }
	}

	return Math::Float(1) / Math::Float(nonZeroProbPaths2);

#elif 1

	const int n = fullPath.s + fullPath.t;

	int nonZeroProbPaths = 1;
	for (int i = 0; i <= n; i++)
	{
		if (i == fullPath.s)
		{
			// Already counted
			continue;
		}

		if (i == 0)
		{
			const auto* p0 = fullPath.FullPathVertex(0);
			if (p0->areaLight != nullptr && !p0->geom.degenerated)
			{
				nonZeroProbPaths++;
			}
		}
		else if (i == n)
		{
			const auto* pn = fullPath.FullPathVertex(n-1);
			if (pn->areaCamera != nullptr && !pn->geom.degenerated)
			{
				nonZeroProbPaths++;
			}
		}
		else if (0 < i && i < n)
		{
			// Check if one of connection vertices are degenerated
			// which cannot be sampled by p_i
			const auto* pi     = fullPath.FullPathVertex(i-1);
			const auto* piNext = fullPath.FullPathVertex(i);
			if (!pi->bsdf->Degenerated() && !piNext->bsdf->Degenerated())
			{
				nonZeroProbPaths++;
			}
		}
	}

	int nonZeroProbPaths2 = 1;
	for (int i = 0; i <= n; i++)
	{
		if (i == fullPath.s)
		{
			// Already counted
			continue;
		}

		auto pi = fullPath.EvaluateFullpathPDF(i);
		if (pi > Math::Float(0))
		{
			nonZeroProbPaths2++;
		}
	}

#if LM_DEBUG_MODE
	if (nonZeroProbPaths != nonZeroProbPaths2)
	{
		__debugbreak();
	}
#endif

	return Math::Float(1) / Math::Float(nonZeroProbPaths2);

#else

	const int n = fullPath.s + fullPath.t;

	// This is tricky part, in the weight calculation
	// we need to exclude samples with zero probability.
	// We note that if the last vertex of sub-path with s=0 or t=0
	// is not on non-pinhole camera or area light, the path probability is zero,
	// because we cannot sample degenerated points.
	int nonZeroProbPaths2 = n+1;
	if (fullPath.lightSubpath.vertices[0]->geom.degenerated)
	{
		nonZeroProbPaths2--;
		if (fullPath.s == 0)
		{
			return Math::Float(0);
		}
	}
	if (fullPath.eyeSubpath.vertices[0]->geom.degenerated)
	{
		nonZeroProbPaths2--;
		if (fullPath.t == 0)
		{
			return Math::Float(0);
		}
	}

	return Math::Float(1) / Math::Float(nonZeroProbPaths2);

#endif
}

LM_COMPONENT_REGISTER_IMPL(BPTSimpleMISWeight, BPTMISWeight);

LM_NAMESPACE_END