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

#ifndef __NANON_TEST_STUB_TRIANGLE_MESH_H__
#define __NANON_TEST_STUB_TRIANGLE_MESH_H__

#include "common.h"
#include <nanon/trianglemesh.h>

NANON_NAMESPACE_BEGIN
NANON_TEST_NAMESPACE_BEGIN

class StubTriangleMesh : public TriangleMesh
{
public:

	StubTriangleMesh(const std::string& id) : TriangleMesh(id) {}
	virtual int NumVertices() const { return static_cast<int>(positions.size()); }
	virtual int NumFaces() const { return static_cast<int>(faces.size()); }
	virtual const Math::Vec3* Positions() const { return positions.empty() ? nullptr : &positions[0]; }
	virtual const Math::Vec3* Normals() const { return normals.empty() ? nullptr : &normals[0]; }
	virtual const Math::Vec2* TexCoords() const { return texcoords.empty() ? nullptr : &texcoords[0]; }
	virtual const Math::Vec3i* Faces() const { return faces.empty() ? nullptr : &faces[0]; }
	virtual bool Load( const pugi::xml_node& node, const Assets& assets ) { return false; }
	virtual std::string Type() const { return "stub"; }

protected:

	std::vector<Math::Vec3> positions;
	std::vector<Math::Vec3> normals;
	std::vector<Math::Vec2> texcoords;
	std::vector<Math::Vec3i> faces;

};

NANON_TEST_NAMESPACE_END
NANON_NAMESPACE_END

#endif // __NANON_TEST_STUB_TRIANGLE_MESH_H__