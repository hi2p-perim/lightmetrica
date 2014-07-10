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
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

/*!
	Raw mesh.
	Implements an triangle mesh whose geometry information
	is stored directly in the configuration file.
*/
class RawMesh : public TriangleMesh
{
public:

	LM_COMPONENT_IMPL_DEF("raw");

public:

	RawMesh() {}
	virtual ~RawMesh() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual int NumVertices() const					{ return static_cast<int>(positions.size()); }
	virtual int NumFaces() const					{ return static_cast<int>(faces.size()); }
	virtual const Math::Float* Positions() const	{ return positions.empty() ? nullptr : &positions[0]; }
	virtual const Math::Float* Normals() const		{ return normals.empty() ? nullptr : &normals[0]; }
	virtual const Math::Float* TexCoords() const	{ return texcoords.empty() ? nullptr : &texcoords[0]; }
	virtual const unsigned int* Faces() const		{ return faces.empty() ? nullptr : &faces[0]; }

public:

	std::vector<Math::Float> positions;
	std::vector<Math::Float> normals;
	std::vector<Math::Float> texcoords;
	std::vector<unsigned int> faces;

};

bool RawMesh::Load( const ConfigNode& node, const Assets& assets )
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

LM_COMPONENT_REGISTER_IMPL(RawMesh, TriangleMesh);

LM_NAMESPACE_END
