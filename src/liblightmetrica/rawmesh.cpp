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
#include <pugixml.hpp>

LM_NAMESPACE_BEGIN

class RawMesh::Impl : public Object
{
public:

	Impl(RawMesh* self);

public:

	bool LoadAsset(const pugi::xml_node& node, const Assets& assets);

private:

	void ParseFloatArray(const pugi::xml_node& node, std::vector<Math::Float>& v);
	void ParseUIntArray(const pugi::xml_node& node, std::vector<unsigned int>& v);

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

bool RawMesh::Impl::LoadAsset( const pugi::xml_node& node, const Assets& assets )
{
	// 'positions' required
	auto positionsNode = node.child("positions");
	if (!positionsNode)
	{
		LM_LOG_ERROR("Missing 'positions' element");
		return false;
	}
	ParseFloatArray(positionsNode, positions);

	// 'normals' required
	auto normalsNode = node.child("normals");
	if (!normalsNode)
	{
		LM_LOG_ERROR("Missing 'normals' element");
		return false;
	}
	ParseFloatArray(normalsNode, normals);

	// 'texcoords'
	auto texcoordsNode = node.child("texcoords");
	if (texcoordsNode)
	{
		ParseFloatArray(texcoordsNode, texcoords);
	}

	// 'faces' required
	auto facesNode = node.child("faces");
	if (!facesNode)
	{
		LM_LOG_ERROR("Missing 'faces' element");
		return false;
	}
	ParseUIntArray(facesNode, faces);

	return true;
}

void RawMesh::Impl::ParseFloatArray( const pugi::xml_node& node, std::vector<Math::Float>& v )
{
	std::stringstream ss(node.child_value());
	double t;
	while (ss >> t)
	{
		v.push_back(Math::Float(t));
	}
}

void RawMesh::Impl::ParseUIntArray( const pugi::xml_node& node, std::vector<unsigned int>& v )
{
	std::stringstream ss(node.child_value());
	unsigned int t;
	while (ss >> t)
	{
		v.push_back(t);
	}
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

bool RawMesh::LoadAsset( const pugi::xml_node& node, const Assets& assets )
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
