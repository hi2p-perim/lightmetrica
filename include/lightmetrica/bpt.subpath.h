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
#ifndef LIB_LIGHTMETRICA_BPT_SUBPATH_H
#define LIB_LIGHTMETRICA_BPT_SUBPATH_H

#include "bpt.common.h"
#include "math.types.h"
#include "surfacegeometry.h"
#include "transportdirection.h"
#include "align.h"
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
class BPTPathVertex : public SIMDAlignedType
{
public:

	BPTPathVertex();

private:

	LM_DISABLE_COPY_AND_MOVE(BPTPathVertex);

public:

	/*!
		Debug print.
		Prints the summary of the path vertex.
	*/
	void DebugPrint() const;

public:

	// General information
	BPTPathVertexType type;					//!< Vertex type
	SurfaceGeometry geom;					//!< Surface geometry information

	/*
		Variables associated with Emitter.
		#type is EndPoint
	*/
	Math::PDFEval pdfP;						//!< PDF evaluation for positional component
	const Emitter* emitter;

	/*
		Variables associated with generalized BSDF.
		#type is either EndPoint or IntermediatePoint
	*/
	Math::Vec3 weight;						//!< Value of f_s / p_{\omega^\bot}
	Math::PDFEval pdfD[2];					//!< PDF evaluation for directional component for each transport direction
	Math::PDFEval pdfRR;					//!< PDF evaluation for Russian roulette
	TransportDirection transportDir;		//!< Transport direction
	const GeneralizedBSDF* bsdf;			//!< Generalized BSDF
	const Light* areaLight;					//!< Light associated with surface
	const Camera* areaCamera;				//!< Camera associated with surface
	Math::Vec3 wi;							//!< Incoming ray
	Math::Vec3 wo;							//!< Outgoing ray in #dir

};

class BPTPathVertexPool;
class BPTConfig;
class Scene;
class Sampler;

/*!
	BPT sub-path.
	Represents a light sub-path or eye sub-path.
*/
class BPTSubpath
{
public:

	LM_PUBLIC_API BPTSubpath(TransportDirection transportDir);
	
private:

	LM_DISABLE_COPY_AND_MOVE(BPTSubpath);

public:

	/*!
		Clear.
		Clears sampled sub-path.
	*/
	LM_PUBLIC_API void Clear();

	/*!
		Debug print.
		Prints contents of the sub-path.
	*/
	LM_PUBLIC_API void DebugPrint() const;

	/*!
		Debug print (by # of vertices).
		Print #n vertices.
		\param n Number of vertices.
	*/
	LM_PUBLIC_API void DebugPrint(size_t n) const;

	/*!
		Sample a sub-path.
		Sample eye sub-path or light sub-path according to #transportDir.
		\param config BPT configuration.
		\param scene Scene.
		\param sampler Sampler.
		\param pool Memory pool for path vertex.
	*/
	LM_PUBLIC_API void Sample(const BPTConfig& config, const Scene& scene, Sampler& sampler, BPTPathVertexPool& pool);

	/*!
		Evaluate alpha of sub-paths.
		The function is called from #EvaluateUnweightContribution.
		\param vs Number of vertices in sub-path (#s or #t).
		\param rasterPosition Raster position.
	*/
	Math::Vec3 EvaluateSubpathAlpha(int vs, Math::Vec2& rasterPosition) const;

	/*!
		Get number of vertices.
		This function is used for testing.
		\return Number of vertices.
	*/
	LM_PUBLIC_API int NumVertices() const;

	/*!
		Get i-th vertex.
		This function is used for testing.
		\param i Index.
		\return Vertex.
	*/
	LM_PUBLIC_API BPTPathVertex* GetVertex(int i) const;

public:

	TransportDirection transportDir;
	std::vector<BPTPathVertex*> vertices;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_SUBPATH_H