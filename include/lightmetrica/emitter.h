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
#ifndef LIB_LIGHTMETRICA_EMITTER_H
#define LIB_LIGHTMETRICA_EMITTER_H

#include "generalizedbsdf.h"

LM_NAMESPACE_BEGIN

struct Primitive;
struct SurfaceGeometry;

/*!
	Emitter.
	The base class of Light and Camera.
*/
class LM_PUBLIC_API Emitter : public GeneralizedBSDF
{
public:

	Emitter();
	Emitter(const std::string& id);
	virtual ~Emitter();

public:

	/*!
		Sample a position on the light.
		\param sample Position sample.
		\param geom Surface geometry at sampled position #geom.p.
		\param pdf Evaluated PDF (area measure).
	*/
	virtual void SamplePosition(const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf) const = 0;

	/*!
		Evaluate the positional component of the emitted quantity.
		\param geom Surface geometry.
		\return Positional component of the emitted quantity.
	*/
	virtual Math::Vec3 EvaluatePosition(const SurfaceGeometry& geom) const = 0;

	/*!
		Evaluate positional PDF.
		\param geom Surface geometry.
		\return Evaluated PDF.
	*/
	virtual Math::PDFEval EvaluatePositionPDF(const SurfaceGeometry& geom) const = 0;

	/*!
		Register an reference to the primitive.
		Some implementation of camera needs transformed mesh information for sampling.
		The function registers the reference to the primitive.
		The function is internally called.
		\param primitives An list instances of the primitive.
	*/
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_EMITTER_H