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
#ifndef LIB_LIGHTMETRICA_TEST_STUB_BSDF_H
#define LIB_LIGHTMETRICA_TEST_STUB_BSDF_H

#include "common.h"
#include <lightmetrica/bsdf.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubBSDF : public BSDF
{
public:

	LM_COMPONENT_IMPL_DEF("stub");

public:

	virtual bool Load(const ConfigNode& node, const Assets& assets) { return true; }
	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const { return true; }
	virtual Math::Vec3 SampleAndEstimateDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const { return true; }
	virtual bool SampleAndEstimateDirectionBidir( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleBidirResult& result ) const { return true; }
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const { return Math::Vec3(); }
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const { return Math::PDFEval(); }
	virtual bool Degenerated() const { return false; }
	virtual int BSDFTypes() const { return GeneralizedBSDFType::None; }

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_BSDF_H
