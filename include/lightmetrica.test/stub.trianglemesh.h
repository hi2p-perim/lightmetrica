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

#pragma once
#ifndef LIB_LIGHTMETRICA_TEST_STUB_TRIANGLE_MESH_H
#define LIB_LIGHTMETRICA_TEST_STUB_TRIANGLE_MESH_H

#include "common.h"
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/align.h>
#include <random>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubTriangleMesh : public TriangleMesh
{
public:

	LM_COMPONENT_IMPL_DEF("stub");

public:

	virtual int NumVertices() const { return static_cast<int>(positions.size()); }
	virtual int NumFaces() const { return static_cast<int>(faces.size()); }
	virtual const Math::Float* Positions() const { return positions.empty() ? nullptr : &positions[0]; }
	virtual const Math::Float* Normals() const { return normals.empty() ? nullptr : &normals[0]; }
	virtual const Math::Float* TexCoords() const { return texcoords.empty() ? nullptr : &texcoords[0]; }
	virtual const unsigned int* Faces() const { return faces.empty() ? nullptr : &faces[0]; }
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return false; }

protected:

	std::vector<Math::Float> positions;
	std::vector<Math::Float> normals;
	std::vector<Math::Float> texcoords;
	std::vector<unsigned int> faces;

};

// {(x, y, z) : 0<=x,y<=1, z=0,-1}
class StubTriangleMesh_Simple : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple()
	{
		const double ps[] =
		{
			0, 0, 0,
			1, 0, 0,
			1, 1, 0,
			0, 1, 0,
			0, 0, -1,
			1, 0, -1,
			1, 1, -1,
			0, 1, -1
		};

		const double ns[] =
		{
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1
		};

		const double ts[] =
		{
			0, 0,
			1, 0,
			1, 1,
			0, 1,
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const unsigned int fs[] =
		{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7
		};

		for (int i = 0; i < 8; i++)
		{
			positions.push_back(Math::Float(ps[3*i  ]));
			positions.push_back(Math::Float(ps[3*i+1]));
			positions.push_back(Math::Float(ps[3*i+2]));
			normals.push_back(Math::Float(ns[3*i  ]));
			normals.push_back(Math::Float(ns[3*i+1]));
			normals.push_back(Math::Float(ns[3*i+2]));
			texcoords.push_back(Math::Float(ts[2*i  ]));
			texcoords.push_back(Math::Float(ts[2*i+1]));
		}

		for (int i = 0; i < 4; i++)
		{
			faces.push_back(fs[3*i]);
			faces.push_back(fs[3*i+1]);
			faces.push_back(fs[3*i+2]);
		}
	}

};

// {(x, y, z) : 0<=x,y<=1, x=-z}
class StubTriangleMesh_Simple2 : public StubTriangleMesh
{
public:

	StubTriangleMesh_Simple2()
	{
		const double ps[] =
		{
			0, 0, 0,
			1, 0, -1,
			1, 1, -1,
			0, 1, 0
		};

		const double ts[] =
		{
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};

		const unsigned int fs[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		auto n = Math::Normalize(Math::Vec3(1, 0, 1));

		for (int i = 0; i < 4; i++)
		{
			positions.push_back(Math::Float(ps[3*i  ]));
			positions.push_back(Math::Float(ps[3*i+1]));
			positions.push_back(Math::Float(ps[3*i+2]));
			normals.push_back(n[0]);
			normals.push_back(n[1]);
			normals.push_back(n[2]);
			texcoords.push_back(Math::Float(ts[2*i  ]));
			texcoords.push_back(Math::Float(ts[2*i+1]));
		}

		for (int i = 0; i < 2; i++)
		{
			faces.push_back(fs[3*i]);
			faces.push_back(fs[3*i+1]);
			faces.push_back(fs[3*i+2]);
		}
	}

};

// Many triangles in [0, 1]^3
class StubTriangleMesh_Random : public StubTriangleMesh
{
public:

	StubTriangleMesh_Random()
	{
		// Fix seed
		std::mt19937 gen(42);
		std::uniform_real_distribution<double> dist;

		const int FaceCount = 1000;
		for (int i = 0; i < FaceCount; i++)
		{
			auto p1 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p2 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));
			auto p3 = Math::Vec3(Math::Float(dist(gen)), Math::Float(dist(gen)), Math::Float(dist(gen)));

			positions.push_back(p1[0]);
			positions.push_back(p1[1]);
			positions.push_back(p1[2]);
			positions.push_back(p2[0]);
			positions.push_back(p2[1]);
			positions.push_back(p2[2]);
			positions.push_back(p3[0]);
			positions.push_back(p3[1]);
			positions.push_back(p3[2]);

			auto n = Math::Cross(p2 - p1, p3 - p1);
			for (int j = 0; j < 3; j++)
			{
				normals.push_back(n[0]);
				normals.push_back(n[1]);
				normals.push_back(n[2]);
			}

			faces.push_back(3*i);
			faces.push_back(3*i+1);
			faces.push_back(3*i+2);
		}
	}

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_TRIANGLE_MESH_H
