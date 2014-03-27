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
#include <lightmetrica/dagpt.defaulteval.h>

LM_NAMESPACE_BEGIN

class DAGPTDefaultLightTransportDAGEvaluator::Impl
{
public:

	Impl();

public:

	void EvaluateContribution( const DAGPTLightTransportDAG& dag, const Film& film ) const;

private:



};

DAGPTDefaultLightTransportDAGEvaluator::Impl::Impl()
{
	
}

void DAGPTDefaultLightTransportDAGEvaluator::Impl::EvaluateContribution( const DAGPTLightTransportDAG& dag, const Film& film ) const
{
	//// Enumerate possible light path whose # of vertices is 4 ...
	//// Minimum light path is \bar{L} -> L -> E -> \bar{E}
	//int n = 4;
	//while (true)
	//{
	//	// Enumerate light paths whose # of vertices is #n using
	//	// iterative deepening depth-first search (IDDFS).
	//	

	//	if ()
	//	{

	//	}

	//	n++;
	//}
}

// --------------------------------------------------------------------------------

DAGPTDefaultLightTransportDAGEvaluator::DAGPTDefaultLightTransportDAGEvaluator()
	: p(new Impl)
{

}

DAGPTDefaultLightTransportDAGEvaluator::~DAGPTDefaultLightTransportDAGEvaluator()
{
	LM_SAFE_DELETE(p);
}

void DAGPTDefaultLightTransportDAGEvaluator::EvaluateContribution( const DAGPTLightTransportDAG& dag, const Film& film ) const
{
	p->EvaluateContribution(dag, film);
}

LM_NAMESPACE_END