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

#include "pch.h"
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>

LM_NAMESPACE_BEGIN

/*!
	Thin-lens camera.
	A camera with depth of field support.
*/
class ThinLensCamera : public Camera
{
public:

	LM_COMPONENT_IMPL_DEF("thinlens");

public:

	ThinLensCamera() {}
	~ThinLensCamera() {}

public:

	virtual bool Load( const ConfigNode& node, const Assets& assets );

public:

	virtual bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	virtual Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	virtual bool Degenerated() const { return false; }

public:

	virtual void SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const;
	virtual Math::Vec3 EvaluatePosition( const SurfaceGeometry& geom ) const;
	virtual Math::PDFEval EvaluatePositionPDF( const SurfaceGeometry& geom ) const;
	virtual void RegisterPrimitives( const std::vector<Primitive*>& primitives );

public:

	virtual bool RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const;
	virtual Film* GetFilm() const { return film; }

private:

	Film* film;

};

bool ThinLensCamera::Load( const ConfigNode& node, const Assets& assets )
{
	return false;
}

bool ThinLensCamera::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return false;
}

Math::Vec3 ThinLensCamera::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return Math::Vec3();
}

Math::PDFEval ThinLensCamera::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return Math::PDFEval();
}

void ThinLensCamera::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{

}

Math::Vec3 ThinLensCamera::EvaluatePosition( const SurfaceGeometry& geom ) const
{
	return Math::Vec3();
}

Math::PDFEval ThinLensCamera::EvaluatePositionPDF( const SurfaceGeometry& geom ) const
{
	return Math::PDFEval();
}

void ThinLensCamera::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{

}

bool ThinLensCamera::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	return false;
}

LM_COMPONENT_REGISTER_IMPL(ThinLensCamera, Camera);

LM_NAMESPACE_END