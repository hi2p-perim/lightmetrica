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

LM_NAMESPACE_BEGIN

class Emitter;
class Light;
class Camera;

/*
	BPT path vertex type.
	Vertex type of #PathVertex.
*/
enum class BPTPathVertexType
{
	None,									// Uninitialized
	EndPoint,								// Endpoint (emitter)
	IntermediatePoint,						// Intermediate point (generalized BSDF)
};

/*
	BPT path vertex.
	Represents a light path vertex.
	TODO : Use unrestricted unions in future implementation.
*/
struct BPTPathVertex
{

	// General information
	BPTPathVertexType type;					// Vertex type
	SurfaceGeometry geom;					// Surface geometry information

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
	Math::PDFEval pdfD[2];					// PDF evaluation for directional component for each transport direction
	Math::PDFEval pdfRR;					// PDF evaluation for Russian roulette
	TransportDirection transportDir;		// Transport direction
	const GeneralizedBSDF* bsdf;			// Generalized BSDF
	const Light* areaLight;					// Light associated with surface
	const Camera* areaCamera;				// Camera associated with surface
	Math::Vec3 wi;							// Incoming ray
	Math::Vec3 wo;							// Outgoing ray in #dir

public:

	BPTPathVertex();

public:

	/*!
		Debug print.
		Prints the summary of the path vertex.
	*/
	void DebugPrint() const;

};

// Object pool type for PathVertex.
typedef boost::object_pool<BPTPathVertex, boost_pool_aligned_allocator<std::alignment_of<BPTPathVertex>::value>> PathVertexPool;

/*
	BPT path.
	Represents a light path.
*/
struct BPTPath
{

	std::vector<BPTPathVertex*> vertices;

public:

	void Clear()
	{
		vertices.clear();
	}

	void Add(BPTPathVertex* vertex)
	{
		vertices.push_back(vertex);
	}

	void Release(PathVertexPool& pool)
	{
		for (auto* vertex : vertices)
			pool.destroy(vertex);
		vertices.clear();
	}

	void DebugPrint()
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			LM_LOG_DEBUG("Vertex #" + std::to_string(i));
			LM_LOG_INDENTER();
			vertices[i]->DebugPrint();
		}
	}

};

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

	BPTFullPath(int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath)
		: s(s)
		, t(t)
		, lightSubpath(lightSubpath)
		, eyeSubpath(eyeSubpath)
	{
		LM_ASSERT(s > 0 || t > 0);
		LM_ASSERT(s + t >= 2);
		
		// Compute #pdfDL and #pdfDE
		if (s == 0 && t > 0)
		{
			// Compute #pdfDE[LE]
			auto* z = eyeSubpath.vertices[t-1];
			if (z->areaLight)
			{
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::LE;
				bsdfEQ.type = GeneralizedBSDFType::LightDirection;
				bsdfEQ.wo = z->wi;
				pdfDE[TransportDirection::LE] = z->areaLight->EvaluateDirectionPDF(bsdfEQ, z->geom);
			}
		}
		else if (s > 0 && t == 0)
		{
			// Compute #pdfDL[EL]
			auto* y = lightSubpath.vertices[s-1];
			if (y->areaCamera)
			{
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
				bsdfEQ.wo = y->wi;
				pdfDL[TransportDirection::EL] = y->areaCamera->EvaluateDirectionPDF(bsdfEQ, y->geom);
			}
		}
		else if (s > 0 && t > 0)
		{
			auto* y = lightSubpath.vertices[s-1];
			auto* z = eyeSubpath.vertices[t-1];

			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::All;

			auto yz = Math::Normalize(z->geom.p - y->geom.p);
			auto zy = -yz;

			// Compute #pdfDL[EL]
			if (s > 1)
			{
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.wi = yz;
				bsdfEQ.wo = y->wi;
				pdfDL[TransportDirection::EL] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);
			}

			// Compute #pdfDL[LE]
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = y->wi;
			bsdfEQ.wo = yz;
			pdfDL[TransportDirection::LE] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);

			// Compute #pdfDE[LE]
			if (t > 1)
			{
				bsdfEQ.transportDir = TransportDirection::LE;
				bsdfEQ.wi = zy;
				bsdfEQ.wo = z->wi;
				pdfDE[TransportDirection::LE] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
			}

			// Compute #pdfDE[EL]
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = z->wi;
			bsdfEQ.wo = zy;
			pdfDE[TransportDirection::EL] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}
	}

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_PATH_H