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
#include <lightmetrica/thinlenscamera.h>
#include <lightmetrica/film.h>

LM_NAMESPACE_BEGIN

class ThinLensCamera::Impl : public Object
{
public:

	Impl(ThinLensCamera* self);

public:

	bool LoadAsset( const ConfigNode& node, const Assets& assets );

public:

	bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;
	Math::PDFEval EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

public:

	void SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const;
	Math::Vec3 EvaluatePosition( const SurfaceGeometry& geom ) const;
	Math::PDFEval EvaluatePositionPDF( const SurfaceGeometry& geom ) const;
	void RegisterPrimitives( const std::vector<Primitive*>& primitives );

public:

	bool RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const;
	Film* GetFilm() const { return film; }

private:

	ThinLensCamera* self;
	Film* film;

};

ThinLensCamera::Impl::Impl( ThinLensCamera* self )
	: self(self)
{

}

bool ThinLensCamera::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return false;
}

bool ThinLensCamera::Impl::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return false;
}

Math::Vec3 ThinLensCamera::Impl::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return Math::Vec3();
}

Math::PDFEval ThinLensCamera::Impl::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return Math::PDFEval();
}

void ThinLensCamera::Impl::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{

}

Math::Vec3 ThinLensCamera::Impl::EvaluatePosition( const SurfaceGeometry& geom ) const
{
	return Math::Vec3();
}

Math::PDFEval ThinLensCamera::Impl::EvaluatePositionPDF( const SurfaceGeometry& geom ) const
{
	return Math::PDFEval();
}

void ThinLensCamera::Impl::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{

}

bool ThinLensCamera::Impl::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	return false;
}

// --------------------------------------------------------------------------------

ThinLensCamera::ThinLensCamera( const std::string& id )
	: Camera(id)
	, p(new Impl(this))
{

}

ThinLensCamera::~ThinLensCamera()
{
	LM_SAFE_DELETE(p);
}

bool ThinLensCamera::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	return this->p->RayToRasterPosition(p, d, rasterPos);
}

Film* ThinLensCamera::GetFilm() const
{
	return p->GetFilm();
}

void ThinLensCamera::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	return p->SamplePosition(sample, geom, pdf);
}

Math::Vec3 ThinLensCamera::EvaluatePosition( const SurfaceGeometry& geom ) const
{
	return p->EvaluatePosition(geom);
}

Math::PDFEval ThinLensCamera::EvaluatePositionPDF( const SurfaceGeometry& geom ) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ThinLensCamera::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	return p->RegisterPrimitives(primitives);
}

bool ThinLensCamera::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return p->SampleDirection(query, geom, result);
}

Math::Vec3 ThinLensCamera::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return p->EvaluateDirection(query, geom);
}

Math::PDFEval ThinLensCamera::EvaluateDirectionPDF( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ThinLensCamera::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

LM_NAMESPACE_END