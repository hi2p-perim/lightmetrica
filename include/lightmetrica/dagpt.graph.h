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
#ifndef LIB_LIGHTMETRICA_DAGPT_GRAPH_H
#define LIB_LIGHTMETRICA_DAGPT_GRAPH_H

#include "common.h"
#include "align.h"
#include <vector>

LM_NAMESPACE_BEGIN

/*!
	DAGPT light transport graph vertex.
	Represents a light transport vertex used in DAGPTRenderer.
	The structure is used in both light transport tree and DAG.
*/
struct DAGPTLightTransportGraphVertex
{
	
	

};

/*!
	DAGPT light transport graph edge.
	Represents a light transport edge used in DAGPTRenderer.
	The structures is used in both light transport tree and DAG.
*/
struct DAGPTLightTransportGraphEdge
{

};

/*!
	DAGPT light path tree.
*/
struct DAGPTLightTransportTree
{

};

/*!
	List of light path trees.
*/
class DAGPTLightTransportTrees
{
	std::vector<DAGPTLightTransportTree> trees;
};

class DAGPTMemoryPool;

/*!
	DAGPT light path DAG.
*/
struct DAGPTLightTransportDAG
{

	LM_PUBLIC_API void Release(DAGPTMemoryPool& pool);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DAGPT_GRAPH_H