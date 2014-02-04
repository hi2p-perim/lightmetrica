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
#ifndef __LIB_LIGHTMETRICA_BSDF_H__
#define __LIB_LIGHTMETRICA_BSDF_H__

#include "asset.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

/*!
	Direction of light transport.
	For some BSDF types, the light transport type must be specified.
	For details, see [Veach 1997].
*/
enum TransportDirection
{
	CameraToLight = 0,		//!< Transport direction is from camera to light (a.k.a. radiance transport, or non-adjoint transport)
	LightToCamera = 1,		//!< Transport direction is from light to camera (a.k.a. importance transport, or adjoint transport)
};

/*!
	BSDF type.
	BSDF types of the surface interaction.
*/
enum BSDFType
{
	// Primitive BSDF types
	DiffuseReflection		= 1<<0,
	DiffuseTransmission		= 1<<1,
	SpecularReflection		= 1<<2,
	SpecularTransmission	= 1<<3,
	GlossyReflection		= 1<<4,
	GlossyTransmission		= 1<<5,

	// Useful flags
	Diffuse					= DiffuseReflection | DiffuseTransmission,
	Specular				= SpecularReflection | SpecularTransmission,
	Glossy					= GlossyReflection | GlossyTransmission,
	All						= Diffuse | Specular | Glossy,
	Reflection				= DiffuseReflection | SpecularReflection | GlossyReflection,
	Transmission			= DiffuseTransmission | SpecularTransmission | GlossyTransmission,
};

/*!
	BSDF sample query.
	Query structure for BSDF::SampleWo.
*/
struct BSDFSampleQuery
{
	Math::Vec2 sample;					//!< Uniform random numbers for sampling BSDF.
	int type;							//!< Requested BSDF type.
	TransportDirection transportDir;	//!< Transport direction.
	Math::Vec3 wi;						//!< Input direction in shading coordinates.
};

/*!
	BSDF sample result.
	Sampled data of BSDF::SampleWo.
*/
struct BSDFSampleResult
{
	int sampledType;					//!< Sampled BSDF type.
	Math::Vec3 wo;						//!< Sampled outgoing direction in shading coordinates.
	Math::PDFEval pdf;					//!< Evaluated PDF. We note that some BSDFs, the PDF cannot be explicitly evaluated.
};

/*!
	BSDF evaluate query.
	Query structure for BSDF::Evaluate.
*/
struct BSDFEvaluateQuery
{

	BSDFEvaluateQuery() {}
	BSDFEvaluateQuery(const BSDFSampleQuery& query, const BSDFSampleResult& result)
		: type(query.type)
		, transportDir(query.transportDir)
		, wi(query.wi)
		, wo(result.wo)
	{}

	int type;							//!< Requested BSDF type.
	TransportDirection transportDir;	//!< Transport direction.
	Math::Vec3 wi;						//!< Input direction in shading coordinates.
	Math::Vec3 wo;						//!< Outgoing direction in shading coordinates.

};

struct Intersection;

/*!
	BSDF.
	A base class for BSDF implementations.
*/
class LM_PUBLIC_API BSDF : public Asset
{
public:

	BSDF(const std::string& id);
	virtual ~BSDF();

public:

	std::string Name() const { return "bsdf"; }
	
public:

	/*!
		Get BSDF type.
		\return BSDF type.
	*/
	virtual BSDFType GetBSDFType() const = 0;
	
	/*!
		Sample outgoing vector.
		Given the input direction originated from the point on the surface #wi,
		the function samples outgoing vector #wo from the suited distribution in the solid angle measure.
		\param query Query structure.
		\param sampled Sampled data.
		\retval true Succeeded to sample #wo.
		\retval false Failed to sample #wo.
	*/
	virtual bool Sample(const BSDFSampleQuery& query, BSDFSampleResult& result) const = 0;

	/*!
		Evaluate BSDF.
		Evaluate f_s(w_i, w_o).
		\param query Query structure.
		\param isect Intersection data.
		\return Evaluated contribution.
	*/
	virtual Math::Vec3 Evaluate(const BSDFEvaluateQuery& query, const Intersection& isect) const = 0;

	/*!
		Evaluate PDF.
		Evaluate pdf(w_i, w_o).
		\param query Query structure.
		\return Evaluated PDF.
	*/
	virtual Math::PDFEval Pdf(const BSDFEvaluateQuery& query) const = 0;

protected:

	/*!
	*/
	Math::Float ShadingNormalCorrectionFactor(const BSDFEvaluateQuery& query, const Intersection& isect) const;

};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_BSDF_H__