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
#ifndef LIB_LIGHTMETRICA_GENERALIZED_BSDF_H
#define LIB_LIGHTMETRICA_GENERALIZED_BSDF_H

#include "asset.h"
#include "math.types.h"
#include "transportdirection.h"

LM_NAMESPACE_BEGIN

/*!
	Generalized BSDF type.
	BSDF types of the surface interaction.
*/
enum GeneralizedBSDFType
{

	// Uninitialized value
	None					= 0,

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
	Query structure for SampleDirection and SampleAndEstimateDirection.
*/
struct GeneralizedBSDFSampleQuery
{

	int type;							//!< Requested BSDF type(s).
	Math::Vec2 sample;					//!< Uniform random numbers for sampling BSDF.
	Math::Float uComp;					//!< Uniform random number for component selection.
	TransportDirection transportDir;	//!< Transport direction.
	Math::Vec3 wi;						//!< Input direction in world coordinates.

};

/*!
	Sampled result for generalized BSDF.
	Sampled result of SampleDirection and SampleAndEstimateDirection.
*/
struct GeneralizedBSDFSampleResult
{

	int sampledType;					//!< Sampled BSDF type.
	Math::Vec3 wo;						//!< Sampled outgoing direction in world coordinates.
	Math::PDFEval pdf;					//!< Evaluated PDF. We note that some BSDFs, the PDF cannot be explicitly evaluated.

};

/*!
	Sampled result for generalized BSDF.
	Sampled result of SampleAndEstimateDirectionBidir.
*/
struct GeneralizedBSDFSampleBidirResult
{

	int sampledType;					//!< Sampled BSDF type.
	Math::Vec3 wo;						//!< Sampled outgoing direction in world coordinates.
	Math::Vec3 weight[2];				//!< Evaluated weights.
	Math::PDFEval pdf[2];				//!< Evaluated PDFs.

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

	GeneralizedBSDFEvaluateQuery(int type, TransportDirection transportDir, const Math::Vec3& wi, const Math::Vec3& wo)
		: type(type)
		, transportDir(transportDir)
		, wi(wi)
		, wo(wo)
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
class GeneralizedBSDF : public Asset
{
public:

	GeneralizedBSDF() {}
	virtual ~GeneralizedBSDF() {}

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
		Sample and estimate direction.
		Computes f_s / p_{\omega^\bot}.
		\param query Query structure.
		\param geom Surface geometry.
		\param result Sampled result.
		\return Estimate.
	*/
	virtual Math::Vec3 SampleAndEstimateDirection(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result) const = 0;

	/*!
		Sample and estimate direction bidirectionally.
		In addition to SampleAndEstimateDirection, this function calculates values in the opposite direction.
		This function is introduced in order to avoid nasty precision problem with specular BSDFs.
		\param query Query structure.
		\param geom Surface geometry.
		\param weights Estimate.
		\param results Sampled results.
		\retval true Succeeded to sample.
		\retval false Failed to sample.
	*/
	virtual bool SampleAndEstimateDirectionBidir(const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result) const = 0;

	/*!
		Evaluate generalized BSDF.
		\param query Query structure.
		\param geom Surface geometry.
		\return Evaluated contribution.
	*/
	virtual Math::Vec3 EvaluateDirection(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const = 0;
	
	/*!
		Evaluate directional PDF.
		\param query Query structure.
		\param geom Surface geometry.
		\return Evaluated PDF.
	*/
	virtual Math::PDFEval EvaluateDirectionPDF(const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom) const = 0;

	/*!
		Check if generalized BSDF is directionally degenerated.
		e.g. specular BSDFs or directional light
		\retval true The BSDF is directionally degenerated.
		\retval false The BSDF is not directionally degenerated.
	*/
	virtual bool Degenerated() const = 0;

	/*!
		Get generalized BSDF type.
		If the generalized BSDF has multiple types, e.g. SpecularReflection and SpecularReflection,
		a bitmask of these types are returned.
		\return Types.
	*/
	virtual int BSDFTypes() const = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_GENERALIZED_BSDF_H