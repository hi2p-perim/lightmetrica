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
#ifndef LIB_LIGHTMETRICA_TRIANGLE_MESH_H
#define LIB_LIGHTMETRICA_TRIANGLE_MESH_H

#include "asset.h"
#include "math.types.h"
#include <vector>

LM_NAMESPACE_BEGIN

/*!
	Triangle mesh.
	A base class for the triangle meshes.
*/
class TriangleMesh : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("triangle_mesh", "triangle_meshes");
	LM_ASSET_NO_DEPENDENCIES();

public:

	TriangleMesh() {}
	virtual ~TriangleMesh() {}

public:

	/*!
		Get the number of vertices.
		\return The number of vertices.
	*/
	virtual int NumVertices() const = 0;

	/*!
		Get the number of faces.
		\return The number of faces.
	*/
	virtual int NumFaces() const = 0;

	/*!
		Get the position array.
		\return The position array.
	*/
	virtual const Math::Float* Positions() const = 0;

	/*!
		Get the normal array.
		\return The normal array.
	*/
	virtual const Math::Float* Normals() const = 0;

	/*!
		Get the texture coordinates array.
		\return The texture coordinates array.
	*/
	virtual const Math::Float* TexCoords() const = 0;

	/*!
		Get the face array.
		\return The face array.
	*/
	virtual const unsigned int* Faces() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TRIANGLE_MESH_H