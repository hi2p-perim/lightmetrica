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
#ifndef LIB_LIGHTMETRICA_DIELECTRIC_H
#define LIB_LIGHTMETRICA_DIELECTRIC_H

#include "bsdf.h"

LM_NAMESPACE_BEGIN

/*!
	Dielectric BSDF.
	Implements dielectric BSDF.
*/
class LM_PUBLIC_API DielectricBSDF : public BSDF
{
public:

	DielectricBSDF(const std::string& id);
	~DielectricBSDF();

public:

	virtual std::string Type() const { return "dielectric"; }

public:

	virtual bool LoadAsset( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

private:

	class Impl;
	Impl* p;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_DIELECTRIC_H