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
#ifndef LIB_LIGHTMETRICA_DAGPT_POOL_H
#define LIB_LIGHTMETRICA_DAGPT_POOL_H

#include "dagpt.graph.h"

#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

LM_NAMESPACE_BEGIN

struct DAGPTLightTransportGraphVertex;
struct DAGPTLightTransportGraphEdge;

/*!
	Memory pool for DAGPT.
	Offers a memory pool implementation for tree vertex and edge.
*/
class LM_PUBLIC_API DAGPTMemoryPool
{
public:

	DAGPTMemoryPool();
	virtual ~DAGPTMemoryPool();

private:

	LM_DISABLE_COPY_AND_MOVE(DAGPTMemoryPool);

public:

	/*!
	*/
	DAGPTLightTransportGraphVertex* AllocateVertex();

	/*!
	*/
	DAGPTLightTransportGraphEdge* AllocateEdge();

	/*!
	*/
	void Release(DAGPTLightTransportGraphVertex* vertex);

	/*!
	*/
	void Release(DAGPTLightTransportGraphEdge* edge);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DAGPT_POOL_H