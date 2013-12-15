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

#ifndef __LIB_NANON_PRIMITIVE_H__
#define __LIB_NANON_PRIMITIVE_H__

#include "common.h"

NANON_NAMESPACE_BEGIN

class TriangleMesh;
class Material;
class Camera;
class Light;

/*!
	Primitive.
	Primitive is an element of the scene used for managing ray-intersectable objects,
	i.e., an object which associated with a triangle mesh.
	We note that some cameras or lights are associated with triangle mesh.
	A primitive corresponds to a node in the scene.
*/
class NANON_PUBLIC_API Primitive
{
public:

	TriangleMesh* mesh;
	Material* material;
	Camera* camera;
	Light* light;

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_PRIMITIVE_H__