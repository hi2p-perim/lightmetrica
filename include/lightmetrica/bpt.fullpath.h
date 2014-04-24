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
#ifndef LIB_LIGHTMETRICA_BPT_FULL_PATH_H
#define LIB_LIGHTMETRICA_BPT_FULL_PATH_H

#include "bpt.common.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

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
	BPTFullPath(int s, int t, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath);

private:

	LM_DISABLE_COPY_AND_MOVE(BPTFullPath);

public:

	/*!
		Evaluate unweight contribution C^*_{s,t}.
		\param scene Scene.
		\param rasterPosition Raster position.
		\return Contribution.
	*/
	Math::Vec3 EvaluateUnweightContribution(const Scene& scene, Math::Vec2& rasterPosition) const;

public:

	const BPTSubpath& lightSubpath;		//!< Light sub-path
	const BPTSubpath& eyeSubpath;		//!< Eye sub-path
	int s;								//!< # of vertices in light sub-path
	int t;								//!< # of vertices in eye-subpath
	Math::PDFEval pdfDL[2];				//!< PDF evaluation for y_{s-1}
	Math::PDFEval pdfDE[2];				//!< PDF evaluation for z_{t-1}

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BPT_FULL_PATH_H