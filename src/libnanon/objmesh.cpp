/*
	nanon : A research-oriented renderer

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
#include <nanon/objmesh.h>
#include <nanon/logger.h>
#include <nanon/pugihelper.h>
#include <pugixml.hpp>
#include <tiny_obj_loader.h>

NANON_NAMESPACE_BEGIN

class ObjMesh::Impl
{
public:

	Impl(ObjMesh* self);
	bool Load(const pugi::xml_node& node);

public:

	ObjMesh* self;
	std::vector<Math::Vec3> positions;
	std::vector<Math::Vec3> normals;
	std::vector<Math::Vec2> texcoords;

};

ObjMesh::Impl::Impl( ObjMesh* self )
	: self(self)
{

}

bool ObjMesh::Impl::Load( const pugi::xml_node& node )
{
	// Check name and type
	if (node.name() != self->Name())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid node name '%s'") % node.name()));
		return false;
	}

	if (node.attribute("type").as_string() != self->Type())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid triangle mesh type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// Find 'path' element
	auto pathNode = node.child("path");
	if (!pathNode)
	{
		NANON_LOG_ERROR("Missing 'path' element");
		return false;
	}

	// Load obj file
	std::string path = pathNode.child_value();
	std::vector<tinyobj::shape_t> shapes;

	auto error = tinyobj::LoadObj(shapes, path.c_str());
	if (!error.empty())
	{
		NANON_LOG_ERROR(error);
		return false;
	}

	NANON_LOG_INFO(boost::str(boost::format("Loaded obj file : %s") % path));

	// Clear current data
	positions.clear();
	normals.clear();
	texcoords.clear();

	// Load shapes
	// Polygons are automatically triangulated
	auto convToFloat = [](float v){ return Math::Float(v); };
	for (const auto& shape : shapes)
	{
		const auto& p = shape.mesh.positions;
		const auto& n = shape.mesh.normals;
		const auto& t = shape.mesh.texcoords;
		std::transform(p.begin(), p.end(), std::back_inserter(positions), convToFloat);
		std::transform(n.begin(), n.end(), std::back_inserter(normals), convToFloat);
		std::transform(t.begin(), t.end(), std::back_inserter(texcoords), convToFloat);
	}

	return true;
}

// --------------------------------------------------------------------------------

ObjMesh::ObjMesh( const std::string& id )
	: TriangleMesh(id)
	, p(new Impl(this))
{

}

ObjMesh::~ObjMesh()
{
	NANON_SAFE_DELETE(p);
}

bool ObjMesh::Load( const pugi::xml_node& node )
{
	return p->Load(node);
}

int ObjMesh::NumFaces() const
{
	return static_cast<int>(p->positions.size()) / 3;
}

const Math::Vec3* ObjMesh::Positions() const
{
	return &p->positions[0];
}

const Math::Vec3* ObjMesh::Normals() const
{
	return &p->normals[0];
}

const Math::Vec2* ObjMesh::TexCoords() const
{
	return &p->texcoords[0];
}

NANON_NAMESPACE_END