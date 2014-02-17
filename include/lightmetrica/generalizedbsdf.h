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
#ifndef __LIB_LIGHTMETRICA_GENERALIZED_BSDF_H__
#define __LIB_LIGHTMETRICA_GENERALIZED_BSDF_H__

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

	/*!
		Transport direction is from camera to light,
		a.k.a. radiance transport, or non-adjoint transport.
	*/
	EL = 0,		

	/*!
		Transport direction is from light to camera,
		a.k.a. importance transport, or adjoint transport.
	*/
	LE = 1,

};

/*!
	Generalized BSDF type.
	BSDF types of the surface interaction.
*/
enum GeneralizedBSDFType
{

	// Primitive BSDF types
	DiffuseReflection		= 1<<0,
	DiffuseTransmission		= 1<<1,
	SpecularReflection		= 1<<2,
	SpecularTransmission	= 1<<3,
	GlossyReflection		= 1<<4,
	GlossyTransmission		= 1<<5,

	// Emitter types
	LightDirection			= 1<<6,
	EyeDirection			= 1<<7,

	// Material type flags for BSDF types
	Diffuse					= DiffuseReflection | DiffuseTransmission,
	Specular				= SpecularReflection | SpecularTransmission,
	Glossy					= GlossyReflection | GlossyTransmission,
	Reflection				= DiffuseReflection | SpecularReflection | GlossyReflection,
	Transmission			= DiffuseTransmission | SpecularTransmission | GlossyTransmission,

	// Useful type flags
	AllEmitter				= LightDirection | EyeDirection,
	AllBSDF					= Diffuse | Specular | Glossy,
	All						= AllEmitter | AllBSDF,

};

/*!
	Sample query for generalized BSDF.
	Query structure for GeneralizedBSDF::SampleDirection.
*/
struct GeneralizedBSDFSampleQuery
{

	int type;							//!< Requested BSDF type(s).
	Math::Vec2 sample;					//!< Uniform random numbers for sampling BSDF.
	TransportDirection transportDir;	//!< Transport direction.
	Math::Vec3 wi;						//!< Input direction in world coordinates.

};

/*!
	Sampled result for generalized BSDF.
	Sampled result of GeneralizedBSDF::SampleDirection.
*/
struct GeneralizedBSDFSampleResult
{

	int sampledType;					//!< Sampled BSDF type.
	Math::Vec3 wo;						//!< Sampled outgoing direction in world coordinates.
	Math::PDFEval pdf;					//!< Evaluated PDF. We note that some BSDFs, the PDF cannot be explicitly evaluated.

};

/*!
	Evaluate query for generalized BSDF.
	Query structure for GeneralizedBSDF::EvaluateDirection.
*/
struct GeneralizedBSDFEvaluateQuery
{

	GeneralizedBSDFEvaluateQuery() {}
	GeneralizedBSDFEvaluateQuery(const GeneralizedBSDFSampleQuery& query, const GeneralizedBSDFSampleResult& result)
		: type(result.sampledType)
		, transportDir(query.transportDir)
		, wi(query.wi)
		, wo(result.wo)
	{}

	int type;							//!< Requested BSDF type.
	TransportDirection transportDir;	//!< Transport direction.
	Math::Vec3 wi;						//!< Input direction in shading coordinates.
	Math::Vec3 wo;						//!< Outgoing direction in shading coordinates.

};

struct SurfaceGeometry;

/*!
	Generalized BSDF.
	Offers interfaces for direction sampling and evaluation.
	Thanks to the class BSDF and direction component of light and camera
	can be sampled in the similar way.
*/
class LM_PUBLIC_API GeneralizedBSDF : public Asset
{
public:

	GeneralizedBSDF(const std::string& id);
	virtual ~GeneralizedBSDF();

public:

	/*!
		Sample outgoing vector.
		Given the input direction originated from the point on the surface #wi,
		the function samples outgoing vector #wo from the suited distribution in the solid angle measure.
		\param query Query structure.
		\param geom Surface geometry. 
		\param result Sampled result.
		\retval true Succeeded to sample #wo.
		\retval false Failed to sample #wo.
	*/
	virtual bool SampleDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const = 0;
	
	/*!
		Evaluate generalized BSDF.
		\param query Query structure.
		\param geom Surface geometry.
		\return Evaluated contribution.
	*/
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const = 0;
	
};

LM_NAMESPACE_END

#endif // __LIB_LIGHTMETRICA_GENERALIZED_BSDF_H__