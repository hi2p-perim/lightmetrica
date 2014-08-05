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
	const GeneralizedBSDF* bsdf;			//!< Generalized BSDF (note that BSDF and light or camera can point to different instances).
	const Light* areaLight;					//!< Light associated with surface
	const Camera* areaCamera;				//!< Camera associated with surface
	Math::Vec3 wi;							//!< Incoming ray direction
	Math::Vec3 wo;							//!< Outgoing ray direction

};

class BPTPathVertexPool;
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
		Sample a subpath.
		Sample eye subpath or light subpath according to #transportDir.
		\param scene Scene.
		\param sampler Sampler.
		\param pool Memory pool for path vertex.
		\param rrDepth Depth to begin Russian roulette.
		\param maxPathVertices Maximum number of vertex of subpath.
	*/
	LM_PUBLIC_API void Sample(const Scene& scene, Sampler& sampler, BPTPathVertexPool& pool, int rrDepth, int maxPathVertices);

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