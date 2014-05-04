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
