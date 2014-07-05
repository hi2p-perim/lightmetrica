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
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/align.h>

LM_NAMESPACE_BEGIN

class BPTPathVertexPool::Impl
{
public:

	Impl() {}

public:

	BPTPathVertex* Construct()
	{
		pool.push_back(new BPTPathVertex);
		return pool.back();
	}

	void Release()
	{
		for (auto* v : pool) { LM_SAFE_DELETE(v); }
		pool.clear();
	}

private:

	std::vector<BPTPathVertex*> pool;

};

// --------------------------------------------------------------------------------

BPTPathVertexPool::BPTPathVertexPool()
	: p(new Impl)
{

}

BPTPathVertexPool::~BPTPathVertexPool()
{
	LM_SAFE_DELETE(p);
}

BPTPathVertex* BPTPathVertexPool::Construct()
{
	return p->Construct();
}

void BPTPathVertexPool::Release()
{
	p->Release();
}

LM_NAMESPACE_END