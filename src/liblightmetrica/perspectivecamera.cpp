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
#include <lightmetrica/perspectivecamera.h>
#include <lightmetrica/film.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/math.functions.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/surfacegeometry.h>

LM_NAMESPACE_BEGIN

class PerspectiveCamera::Impl : public Object
{
public:

	Impl(PerspectiveCamera* self);

public:

	bool LoadAsset(const ConfigNode& node, const Assets& assets);

public:

	bool SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const;
	Math::Vec3 EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const;

public:

	void SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const;
	Math::Vec3 EvaluatePosition( const SurfaceGeometry& geom ) const;
	void RegisterPrimitives( const std::vector<Primitive*>& primitives );
	bool RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const;
	Film* GetFilm() const;

private:

	/*
		Calculate importance W_e(z_0\to y_{s-1}),
		i.e., sensitivity of the sensor
	*/
	Math::Float EvaluateImportance(Math::Float cosTheta) const;

private:

	PerspectiveCamera* self;
	Film* film;

	Math::Float invA;
	Math::Vec3 position;
	Math::Mat4 viewMatrix;
	Math::Mat4 invViewMatrix;
	Math::Mat4 projectionMatrix;
	Math::Mat4 invProjectionMatrix;

};

PerspectiveCamera::Impl::Impl( PerspectiveCamera* self )
	: self(self)
{

}

bool PerspectiveCamera::Impl::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	// Resolve reference to film
	film = dynamic_cast<Film*>(assets.ResolveReferenceToAsset(node.Child("film"), "film"));
	if (!film)
	{
		return false;
	}

	// 'fovy'
	Math::Float fovy;
	if (!node.ChildValue("fovy", fovy)) return false;

	// Aspect ratio
	Math::Float aspect = Math::Float(film->Width()) / Math::Float(film->Height());

	// --------------------------------------------------------------------------------

	// Projection matrix and its inverse
	projectionMatrix = Math::Perspective(fovy, aspect, Math::Float(1), Math::Float(1000));
	invProjectionMatrix = Math::Inverse(projectionMatrix);

	// Calculate area of the sensor used for SampleAndEvaluate
	auto ndcP1 = Math::Vec3(-1, -1, 0);
	auto ndcP2 = Math::Vec3(1, 1, 0);

	auto camP1_4 = invProjectionMatrix * Math::Vec4(ndcP1, 1);
	auto camP2_4 = invProjectionMatrix * Math::Vec4(ndcP2, 1);

	auto camP1 = Math::Vec3(camP1_4) / camP1_4.w;
	auto camP2 = Math::Vec3(camP2_4) / camP1_4.w;

	camP1 /= Math::Vec3(camP1.z);
	camP2 /= Math::Vec3(camP2.z);

	Math::Float A = (camP2.x - camP1.x) * (camP2.y - camP1.y);
	invA = Math::Float(1) / A;

	return true;
}

void PerspectiveCamera::Impl::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	LM_ASSERT(primitives.size() == 1);

	// View matrix and its inverse
	viewMatrix = primitives.front()->transform;
	invViewMatrix = Math::Inverse(viewMatrix);

	// Position of the camera (in world coordinates)
	position = Math::Vec3(invViewMatrix * Math::Vec4(0, 0, 0, 1));
}

void PerspectiveCamera::Impl::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	geom.p = position;
	geom.degenerated = true;
	pdf = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Area);
}

bool PerspectiveCamera::Impl::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	if ((query.type & GeneralizedBSDFType::EyeDirection) == 0)
	{
		return false;
	}

	// Raster position in [-1, 1]^2
	auto ndcRasterPos = Math::Vec3(query.sample * Math::Float(2) - Math::Vec2(Math::Float(1)), Math::Float(0));

	// Convert raster position to camera coordinates
	auto dirTCam4 = invProjectionMatrix * Math::Vec4(ndcRasterPos, Math::Float(1));
	auto dirTCam3 = Math::Normalize(Math::Vec3(dirTCam4) / dirTCam4.w);

	result.sampledType = GeneralizedBSDFType::EyeDirection;
	result.wo = Math::Normalize(Math::Vec3(invViewMatrix * Math::Vec4(dirTCam3, Math::Float(0))));
	result.pdf = Math::PDFEval(
		EvaluateImportance(-Math::CosThetaZUp(dirTCam3)),
		Math::ProbabilityMeasure::ProjectedSolidAngle);

	return true;
}

Math::Vec3 PerspectiveCamera::Impl::EvaluatePosition( const SurfaceGeometry& /*geom*/ ) const
{
	return Math::Vec3(Math::Float(1));
}

Math::Vec3 PerspectiveCamera::Impl::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	if ((query.type & GeneralizedBSDFType::LightDirection) != 0)
	{
		return Math::Vec3();
	}

	// Reference point in camera coordinates
	auto refCam4 = viewMatrix * Math::Vec4(geom.p + query.wo, Math::Float(1));
	auto refCam3 = Math::Vec3(refCam4);

	// Reference point in NDC
	auto refNdc4 = projectionMatrix * refCam4;
	auto refNdc3 = Math::Vec3(refNdc4) / refNdc4.w;

	// Importance
	return Math::Vec3(EvaluateImportance(-Math::CosThetaZUp(Math::Normalize(refCam3))));
}

bool PerspectiveCamera::Impl::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	// Reference point in camera coordinates
	auto refCam4 = viewMatrix * Math::Vec4(p + d, Math::Float(1));
	auto refCam3 = Math::Vec3(refCam4);

	// Reference point in NDC
	auto refNdc4 = projectionMatrix * refCam4;
	auto refNdc3 = Math::Vec3(refNdc4) / refNdc4.w;

	// Raster position in [0, 1]^2
	rasterPos = (Math::Vec2(refNdc3.x, refNdc3.y) + Math::Vec2(Math::Float(1))) / Math::Float(2);

	// Check visibility
	if (rasterPos.x < 0 || rasterPos.x > 1 || rasterPos.y < 0 || rasterPos.y > 1)
	{
		return false;
	}

	return true;
}

Math::Float PerspectiveCamera::Impl::EvaluateImportance( Math::Float cosTheta ) const
{
	// Assume hypothetical sensor on z=-d in camera coordinates.
	// Then the sensitivity is 1/Ad^2 where A is area of the sensor when d=1.
	// Converting the measure, the sensitivity is
	//  W_e(z_0\to y_{s-1})
	//   = dA/d\omega 1/Ad^2
	//   = \| p - z_0 \|^2 / \cos{(\theta)} / Ad^2
	//   = 1 / (A * \cos^3{(\theta)}),
	// where p is the raster position on the sensor,
	// \theta is the angle between the normal on p and p - z_0.

	if (cosTheta <= Math::Float(0))
	{
		// p is on behind the camera
		return Math::Float(0);
	}

	Math::Float invCosTheta = Math::Float(1) / cosTheta;
	return invA * invCosTheta * invCosTheta * invCosTheta;
}

// --------------------------------------------------------------------------------

PerspectiveCamera::PerspectiveCamera( const std::string& id )
	: Camera(id)
	, p(new Impl(this))
{
	
}

PerspectiveCamera::~PerspectiveCamera()
{
	LM_SAFE_DELETE(p);
}

Film* PerspectiveCamera::GetFilm() const
{
	return p->GetFilm();
}

bool PerspectiveCamera::LoadAsset( const ConfigNode& node, const Assets& assets )
{
	return p->LoadAsset(node, assets);
}

bool PerspectiveCamera::SampleDirection( const GeneralizedBSDFSampleQuery& query, const SurfaceGeometry& geom, GeneralizedBSDFSampleResult& result ) const
{
	return p->SampleDirection(query, geom, result);
}

Math::Vec3 PerspectiveCamera::EvaluateDirection( const GeneralizedBSDFEvaluateQuery& query, const SurfaceGeometry& geom ) const
{
	return p->EvaluateDirection(query, geom);
}

void PerspectiveCamera::SamplePosition( const Math::Vec2& sample, SurfaceGeometry& geom, Math::PDFEval& pdf ) const
{
	p->SamplePosition(sample, geom, pdf);
}

Math::Vec3 PerspectiveCamera::EvaluatePosition( const SurfaceGeometry& geom ) const
{
	return p->EvaluatePosition(geom);
}

void PerspectiveCamera::RegisterPrimitives( const std::vector<Primitive*>& primitives )
{
	return p->RegisterPrimitives(primitives);
}

bool PerspectiveCamera::RayToRasterPosition( const Math::Vec3& p, const Math::Vec3& d, Math::Vec2& rasterPos ) const
{
	return this->p->RayToRasterPosition(p, d, rasterPos);
}

LM_NAMESPACE_END