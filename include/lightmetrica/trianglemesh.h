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
class LM_PUBLIC_API TriangleMesh : public Asset
{
public:

	TriangleMesh(const std::string& id);
	virtual ~TriangleMesh();

public:

	std::string Name() const { return "triangle_mesh"; }

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