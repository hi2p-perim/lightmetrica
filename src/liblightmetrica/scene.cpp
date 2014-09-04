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

#include "pch.h"
#include <lightmetrica/scene.h>
#include <lightmetrica/config.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/primitives.h>

LM_NAMESPACE_BEGIN

Scene::Scene()
{

}

Scene::~Scene()
{

}

void Scene::Load( Primitives* primitives )
{
	this->primitives.reset(primitives);
}

bool Scene::PostConfigure()
{
	return primitives->PostConfigure(*this);
}

bool Scene::Intersect( Ray& ray, Intersection& isect ) const
{
	// # Intersection with triangles
	if (!IntersectTriangles(ray, isect))
	{
		return false;
	}

	// # Intersection with emitter shapes
	if (!primitives->IntersectEmitterShapes(ray, isect))
	{
		return false;
	}

	return true;
}

const Camera* Scene::MainCamera() const
{
	return primitives->MainCamera();
}

const Light* Scene::SampleLightSelection( Math::Vec2& lightSampleP, Math::PDFEval& selectionPdf ) const
{
	int nl = primitives->NumLights();
	int li = Math::Min(static_cast<int>(lightSampleP.x * nl), nl - 1);
	lightSampleP.x = lightSampleP.x * nl - Math::Float(li);
	selectionPdf = Math::PDFEval(Math::Float(1) / Math::Float(nl), Math::ProbabilityMeasure::Discrete);
	return primitives->LightByIndex(li);
}

const Light* Scene::SampleLightSelection( const Math::Float& lightSample, Math::PDFEval& selectionPdf ) const
{
	int nl = primitives->NumLights();
	int li = Math::Min(static_cast<int>(lightSample * nl), nl - 1);
	selectionPdf = Math::PDFEval(Math::Float(1) / Math::Float(nl), Math::ProbabilityMeasure::Discrete);
	return primitives->LightByIndex(li);
}

Math::PDFEval Scene::LightSelectionPdf() const
{
	return Math::PDFEval(Math::Float(1) / Math::Float(primitives->NumLights()), Math::ProbabilityMeasure::Discrete);
}

void Scene::StoreIntersectionFromBarycentricCoords( unsigned int primitiveIndex, unsigned int triangleIndex, const Ray& ray, const Math::Vec2& b, Intersection& isect ) const
{
	// Primitive
	isect.primitiveIndex = primitiveIndex;
	isect.triangleIndex = triangleIndex;
	isect.primitive = primitives->PrimitiveByIndex(primitiveIndex);

	const auto* mesh = isect.primitive->mesh;
	const auto* positions = mesh->Positions();
	const auto* normals = mesh->Normals();
	const auto* texcoords = mesh->TexCoords();
	const auto* faces = mesh->Faces();

	// Intersection point
	isect.geom.p = ray.o + ray.d * ray.maxT;

	// Geometry normal
	int v1 = faces[3*triangleIndex  ];
	int v2 = faces[3*triangleIndex+1];
	int v3 = faces[3*triangleIndex+2];
	Math::Vec3 p1(isect.primitive->transform * Math::Vec4(positions[3*v1], positions[3*v1+1], positions[3*v1+2], Math::Float(1)));
	Math::Vec3 p2(isect.primitive->transform * Math::Vec4(positions[3*v2], positions[3*v2+1], positions[3*v2+2], Math::Float(1)));
	Math::Vec3 p3(isect.primitive->transform * Math::Vec4(positions[3*v3], positions[3*v3+1], positions[3*v3+2], Math::Float(1)));
	isect.geom.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));

	// Shading normal
	Math::Vec3 n1(isect.primitive->normalTransform * Math::Vec3(normals[3*v1], normals[3*v1+1], normals[3*v1+2]));
	Math::Vec3 n2(isect.primitive->normalTransform * Math::Vec3(normals[3*v2], normals[3*v2+1], normals[3*v2+2]));
	Math::Vec3 n3(isect.primitive->normalTransform * Math::Vec3(normals[3*v3], normals[3*v3+1], normals[3*v3+2]));
	isect.geom.sn = Math::Normalize(n1 * (Math::Float(1) - b[0] - b[1]) + n2 * b[0] + n3 * b[1]);

	// Texture coordinates
	if (texcoords)
	{
		Math::Vec2 uv1(texcoords[2*v1], texcoords[2*v1+1]);
		Math::Vec2 uv2(texcoords[2*v2], texcoords[2*v2+1]);
		Math::Vec2 uv3(texcoords[2*v3], texcoords[2*v3+1]);
		isect.geom.uv = uv1 * Math::Float(Math::Float(1) - b[0] - b[1]) + uv2 * b[0] + uv3 * b[1];
	}

	// Scene surface is not degenerated
	isect.geom.degenerated = false;

	// Compute tangent space
	isect.geom.ComputeTangentSpace();
}

LM_NAMESPACE_END
