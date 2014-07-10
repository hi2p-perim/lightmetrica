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