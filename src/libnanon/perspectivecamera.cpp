/*
	nanon : A research-oriented renderer

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
#include <nanon/perspectivecamera.h>
#include <nanon/film.h>
#include <nanon/assets.h>
#include <nanon/primitive.h>
#include <nanon/logger.h>
#include <nanon/math.functions.h>
#include <nanon/ray.h>
#include <pugixml.hpp>

NANON_NAMESPACE_BEGIN

class PerspectiveCamera::Impl
{
public:

	Impl(PerspectiveCamera* self);
	bool Load(const pugi::xml_node& node, const Assets& assets);
	void RasterPosToRay(const Math::Vec2& rasterPos, Ray& ray) const;
	const Film* GetFilm() const { return film; }
	void RegisterPrimitive(Primitive* primitive);

private:

	PerspectiveCamera* self;
	Film* film;
	Primitive* primitive;

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

bool PerspectiveCamera::Impl::Load( const pugi::xml_node& node, const Assets& assets )
{
	// Check name and type
	if (node.name() != self->Name())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid node name '%s'") % node.name()));
		return false;
	}

	if (node.attribute("type").as_string() != self->Type())
	{
		NANON_LOG_ERROR(boost::str(boost::format("Invalid camera type '%s'") % node.attribute("type").as_string()));
		return false;
	}

	// Resolve reference to film
	film = dynamic_cast<Film*>(assets.ResolveReferenceToAsset(node.child("film"), "film"));
	if (!film)
	{
		NANON_LOG_DEBUG_EMPTY();
		return false;
	}

	// Find 'fovy'
	auto fovyNode = node.child("fovy");
	if (!fovyNode)
	{
		NANON_LOG_ERROR("Missing 'fovy' element");
		return false;
	}
	Math::Float fovy = Math::Float(boost::lexical_cast<double>(fovyNode.child_value()));

	// Projection matrix and its inverse
	projectionMatrix = Math::Perspective(fovy, Math::Float(film->Width()) / Math::Float(film->Height()), Math::Float(1), Math::Float(1000));
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

void PerspectiveCamera::Impl::RasterPosToRay( const Math::Vec2& rasterPos, Ray& ray ) const
{
	// Raster position in [-1, 1]^2
	auto ndcRasterPos = Math::Vec3(rasterPos * Math::Float(2) - Math::Vec2(Math::Float(1)), Math::Float(0));

	// Convert raster position to camera coordinates
	auto dirTCam4 = invProjectionMatrix * Math::Vec4(ndcRasterPos, Math::Float(1));
	auto dirTCam3 = Math::Normalize(Math::Vec3(dirTCam4) / dirTCam4.w);

	ray.d = Math::Normalize(Math::Vec3(invViewMatrix * Math::Vec4(dirTCam3, Math::Float(0))));
	ray.o = position;
	ray.minT = Math::Float(0);
	ray.maxT = Math::Constants::Inf;
}

void PerspectiveCamera::Impl::RegisterPrimitive( Primitive* primitive )
{
	this->primitive = primitive;

	// View matrix and its inverse
	viewMatrix = primitive->transform;
	invViewMatrix = Math::Inverse(viewMatrix);

	// Position of the camera (in world coordinates)
	position = Math::Vec3(invViewMatrix * Math::Vec4(0, 0, 0, 1));
}

// --------------------------------------------------------------------------------

PerspectiveCamera::PerspectiveCamera( const std::string& id )
	: Camera(id)
	, p(new Impl(this))
{
	
}

PerspectiveCamera::~PerspectiveCamera()
{
	NANON_SAFE_DELETE(p);
}

void PerspectiveCamera::RasterPosToRay( const Math::Vec2& rasterPos, Ray& ray ) const
{
	p->RasterPosToRay(rasterPos, ray);
}

const Film* PerspectiveCamera::GetFilm() const
{
	return p->GetFilm();
}

void PerspectiveCamera::RegisterPrimitive( Primitive* primitive )
{
	return p->RegisterPrimitive(primitive);
}

bool PerspectiveCamera::Load( const pugi::xml_node& node, const Assets& assets )
{
	return p->Load(node, assets);
}

NANON_NAMESPACE_END