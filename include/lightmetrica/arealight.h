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
#ifndef LIB_LIGHTMETRICA_AREA_LIGHT_H
#define LIB_LIGHTMETRICA_AREA_LIGHT_H

#include "light.h"

LM_NAMESPACE_BEGIN

/*!
	Area light.
	Implements an area light source.
*/
class LM_PUBLIC_API AreaLight : public Light
{
public:

	AreaLight();
	AreaLight(const std::string& id);
	~AreaLight();

public:

	virtual std::string Type() const { return "area"; }

public:

	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

public:

	virtual void SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const;
	virtual Math::Vec3 EvaluatePosition( const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluatePositionPDF( const SurfaceGeometry& geom ) const;
	virtual void RegisterPrimitives(const std::vector<Primitive*>& primitives);

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_AREA_LIGHT_H