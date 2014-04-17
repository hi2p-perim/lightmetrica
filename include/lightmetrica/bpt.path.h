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
#ifndef LIB_LIGHTMETRICA_BPT_PATH_H
#define LIB_LIGHTMETRICA_BPT_PATH_H

#include "common.h"
#include "math.types.h"
#include "surfacegeometry.h"
#include "transportdirection.h"
#include <vector>

LM_NAMESPACE_BEGIN

class GeneralizedBSDF;
class Emitter;
class Light;
class Camera;

/*!
	BPT path vertex type.
	Vertex type of #PathVertex.
*/
enum class BPTPathVertexType
{
	None,									//!< Uninitialized
	EndPoint,								//!< Endpoint (emitter)
	IntermediatePoint,						//!< Intermediate point (generalized BSDF)
};

/*!
	BPT path vertex.
	Represents a light path vertex.
	TODO : Use unrestricted unions in future implementation.
*/
struct BPTPathVertex
{

	// General information
	BPTPathVertexType type;					//!< Vertex type
	SurfaceGeometry geom;					//!< Surface geometry information

	/*
		Variables associated with Emitter.
		#type is EndPoint
	*/
	Math::PDFEval pdfP;						// PDF evaluation for positional component
	const Emitter* emitter;

	/*
		Variables associated with generalized BSDF.
		#type is either EndPoint or IntermediatePoint
	*/
	Math::PDFEval pdfD[2];					//!< PDF evaluation for directional component for each transport direction
	Math::PDFEval pdfRR;					//!< PDF evaluation for Russian roulette
	TransportDirection transportDir;		//!< Transport direction
	const GeneralizedBSDF* bsdf;			//!< Generalized BSDF
	const Light* areaLight;					//!< Light associated with surface
	const Camera* areaCamera;				//!< Camera associated with surface
	Math::Vec3 wi;							//!< Incoming ray
	Math::Vec3 wo;							//!< Outgoing ray in #dir

public:

	BPTPathVertex();

public:

	/*!
		Debug print.
		Prints the summary of the path vertex.
	*/
	void DebugPrint() const;

};

// --------------------------------------------------------------------------------

class BPTPathVertexPool;

/*
	BPT path.
	Represents a light path.
*/
struct BPTPath
{

	std::vector<BPTPathVertex*> vertices;

public:

	void Clear();
	void Add(BPTPathVertex* vertex);
	void Release(BPTPathVertexPool& pool);
	void DebugPrint();

};

// --------------------------------------------------------------------------------

/*
	BPT full-path.
	Represents a full-path combining light sub-path and eye sub-path.
*/
struct BPTFullPath
{

	const BPTPath& lightSubpath;	// Light sub-path
	const BPTPath& eyeSubpath;		// Eye sub-path
	int s;							// # of vertices in light sub-path
	int t;							// # of vertices in eye-subpath
	Math::PDFEval pdfDL[2];			// PDF evaluation for y_{s-1}
	Math::PDFEval pdfDE[2];			// PDF evaluation for z_{t-1}

public:

	BPTFullPath(int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath);

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_PATH_H