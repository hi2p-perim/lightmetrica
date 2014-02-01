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
#include <lightmetrica/rawmesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

class RawMesh::Impl : public Object
{
public:

	Impl(RawMesh* self);

public:

	bool LoadAsset(const ConfigNode& node, const Assets& assets);

public:

	RawMesh* self;
	std::vector<Math::Float> positions;
	std::vector<Math::Float> normals;
	std::vector<Math::Float> texcoords;
	std::vector<unsigned int> faces;

};

RawMesh::Impl::Impl( RawMesh* self )
	: self(self)
{

}

bool RawMesh::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	// 'positions' required
	if (!node.ChildValue("positions", positions))
	{
		return false;
	}

	// 'normals' required
	if (!node.ChildValue("normals", normals))
	{
		return false;
	}

	// 'texcoords' optional
	node.ChildValue("texcoords", texcoords);								

	// 'faces' required
	if (!node.ChildValue("faces", faces))
	{
		return false;
	}

	return true;
}


// --------------------------------------------------------------------------------

RawMesh::RawMesh( const std::string& id )
	: TriangleMesh(id)
	, p(new Impl(this))
{

}

RawMesh::~RawMesh()
{
	LM_SAFE_DELETE(p);
}

bool RawMesh::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

int RawMesh::NumVertices() const
{
	return static_cast<int>(p->positions.size());
}

int RawMesh::NumFaces() const
{
	return static_cast<int>(p->faces.size());
}

const Math::Float* RawMesh::Positions() const
{
	return p->positions.empty() ? nullptr : &p->positions[0];
}

const Math::Float* RawMesh::Normals() const
{
	return p->normals.empty() ? nullptr : &p->normals[0];
}

const Math::Float* RawMesh::TexCoords() const
{
	return p->texcoords.empty() ? nullptr : &p->texcoords[0];
}

const unsigned int* RawMesh::Faces() const
{
	return p->faces.empty() ? nullptr : &p->faces[0];
}

LM_NAMESPACE_END
