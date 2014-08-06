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
#ifndef LIB_LIGHTMETRICA_BPT_FULL_PATH_H
#define LIB_LIGHTMETRICA_BPT_FULL_PATH_H

#include "bpt.common.h"
#include "math.types.h"
#include "transportdirection.h"

LM_NAMESPACE_BEGIN

class BPTPathVertex;
class BPTSubpath;
class Scene;

/*!
	BPT full-path.
	Represents a full-path combining light sub-path and eye sub-path.
*/
class BPTFullPath
{
public:

	/*!
		Constructor.
		Constructs a full-path from light sub-path and eye-subpath.
		\param s Number of vertices in light sub-path.
		\param t Number of vertices in eye sub-path.
		\param lightSubpath Light sub-path.
		\param eyeSubpath Eye sub-path.
	*/
	LM_PUBLIC_API BPTFullPath(int s, int t, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath);

private:

	LM_DISABLE_COPY_AND_MOVE(BPTFullPath);

public:

	/*!
		Evaluate unweight contribution C^*_{s,t}.
		\param scene Scene.
		\param rasterPosition Raster position.
		\return Contribution.
	*/
	LM_PUBLIC_API Math::Vec3 EvaluateUnweightContribution(const Scene& scene, Math::Vec2& rasterPosition) const;

	/*!
		Evaluate full-path probability density.
		Evaluate p_i(x_{s,t}) := p_{i,s+t-i}(x_{s,t}).
		\param i Index of PDF.
		\return Evaluated PDF.
	*/
	LM_PUBLIC_API Math::Float EvaluateFullpathPDF(int i) const;

	/*!
		Evaluate full-path probability density ratio.
		Evaluate p_{i+1}(x_{s,t}) / p_i(x_{s,t}).
		See Equation (10.9) in [Veach 1997].
		This function requires p_s(x_{s,t}) be non-zero.
		\param i Index of PDF.
		\return Evaluated PDF.
	*/
	LM_PUBLIC_API Math::Float EvaluateFullpathPDFRatio(int i) const;

	/*!
		Check if p_i(x_{s,t}) is zero.
		\param i Index of PDF.
		\retval true Zero.
		\retval false Non-zero.
	*/
	LM_PUBLIC_API bool FullpathPDFIsZero(int i) const;

	/*!
		Debug print.
		Prints contents of the full-path.
	*/
	LM_PUBLIC_API void DebugPrint() const;

public:

	/*!
		Get i-th vertex of the full-path.
		\param i Index of PDF.
		\return Vertex.
	*/
	LM_PUBLIC_API const BPTPathVertex* FullPathVertex(int i) const;

	/*!
		Get i-th directional PDF evaluation of the full-path.
		\param i Index of PDF.
		\param transportDir Transport direction.
		\return Evaluated PDF.
	*/
	Math::PDFEval FullPathVertexDirectionPDF(int i, TransportDirection transportDir) const;

public:

	int s;								//!< # of vertices in light sub-path
	int t;								//!< # of vertices in eye sub-path
	const BPTSubpath& lightSubpath;		//!< Light sub-path
	const BPTSubpath& eyeSubpath;		//!< Eye sub-path
	Math::PDFEval pdfDL[2];				//!< PDF evaluation for y_{s-1} (light sub-path)
	Math::PDFEval pdfDE[2];				//!< PDF evaluation for z_{t-1} (eye sub-path)

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_FULL_PATH_H
