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
#include <nanon/naivescene.h>
#include <nanon/ray.h>
#include <nanon/intersection.h>
#include <nanon/primitive.h>
#include <nanon/trianglemesh.h>
#include <nanon/math.functions.h>

NANON_NAMESPACE_BEGIN

// TriAccel by Ingo Wald
// We used the implementation in mitsuba with a slight modification
struct TriAccel
{

	uint32_t k;
	Math::Float n_u;
	Math::Float n_v;
	Math::Float n_d;

	Math::Float a_u;
	Math::Float a_v;
	Math::Float b_nu;
	Math::Float b_nv;

	Math::Float c_nu;
	Math::Float c_nv;
	uint32_t shapeIndex;
	uint32_t primIndex;

	// Construct from vertex data. Returns '1' if there was a failure
	NANON_FORCE_INLINE int Load(const Math::Vec3& A, const Math::Vec3& B, const Math::Vec3& C);

	// Fast ray-triangle intersection test
	NANON_FORCE_INLINE bool Intersect(const Ray& ray, Math::Float mint, Math::Float maxt, Math::Float& u, Math::Float& v, Math::Float& t) const;

};

NANON_FORCE_INLINE int TriAccel::Load(const Math::Vec3& A, const Math::Vec3& B, const Math::Vec3& C)
{
	static const int waldModulo[4] = { 1, 2, 0, 1 };

	Math::Vec3 b = C - A;
	Math::Vec3 c = B - A;
	Math::Vec3 N = Math::Cross(c, b);

	k = 0;
	// Determine the largest projection axis
	for (int j = 0; j < 3; j++)
	{
		if (Math::Abs(N[j]) > Math::Abs(N[k]))
		{
			k = j;
		}
	}

	uint32_t u = waldModulo[k];
	uint32_t v = waldModulo[k+1];
	const Math::Float n_k = N[k];
	const Math::Float denom = b[u]*c[v] - b[v]*c[u];

	if (denom == 0)
	{
		k = 3;
		return 1;
	}

	// Pre-compute intersection calculation constants
	n_u   =  N[u] / n_k;
	n_v   =  N[v] / n_k;
	n_d   =  Math::Dot(Math::Vec3(A), N) / n_k;
	b_nu  =  b[u] / denom;
	b_nv  = -b[v] / denom;
	a_u   =  A[u];
	a_v   =  A[v];
	c_nu  =  c[v] / denom;
	c_nv  = -c[u] / denom;

	return 0;
}

NANON_FORCE_INLINE bool TriAccel::Intersect(const Ray& ray, Math::Float mint, Math::Float maxt, Math::Float& u, Math::Float& v, Math::Float& t) const
{
	Math::Float o_u, o_v, o_k, d_u, d_v, d_k;
	switch (k)
	{
		case 0:
			o_u = ray.o[1];
			o_v = ray.o[2];
			o_k = ray.o[0];
			d_u = ray.d[1];
			d_v = ray.d[2];
			d_k = ray.d[0];
			break;

		case 1:
			o_u = ray.o[2];
			o_v = ray.o[0];
			o_k = ray.o[1];
			d_u = ray.d[2];
			d_v = ray.d[0];
			d_k = ray.d[1];
			break;
		
		case 2:
			o_u = ray.o[0];
			o_v = ray.o[1];
			o_k = ray.o[2];
			d_u = ray.d[0];
			d_v = ray.d[1];
			d_k = ray.d[2];
			break;

		default:
			return false;
	}


#ifdef NANON_DEBUG_MODE
	if (d_u * n_u + d_v * n_v + d_k == 0)
	{
		return false;
	}
#endif

	// Calculate the plane intersection (Typo in the thesis?)
	t = (n_d - o_u*n_u - o_v*n_v - o_k) / (d_u * n_u + d_v * n_v + d_k);
	if (t < mint || t > maxt)
	{
		return false;
	}

	// Calculate the projected plane intersection point
	const Math::Float hu = o_u + t * d_u - a_u;
	const Math::Float hv = o_v + t * d_v - a_v;

	// In barycentric coordinates
	u = hv * b_nu + hu * b_nv;
	v = hu * c_nu + hv * c_nv;

	return u >= 0 && v >= 0 && u+v <= 1;
}

// --------------------------------------------------------------------------------

class NaiveScene::Impl : public Object
{
public:

	Impl(NaiveScene* self);
	bool Build();
	bool Intersect(Ray& ray, Intersection& isect);

private:

	NaiveScene* self;
	std::vector<TriAccel> triAccels;

};

NaiveScene::Impl::Impl( NaiveScene* self )
	: self(self)
{

}

bool NaiveScene::Impl::Build()
{
	// Almost do nothing; simply creates a list of triangles from the primitives
	// as data structure, we used Wald's TriAccel.
	
	for (int i = 0; i < self->NumPrimitives(); i++)
	{
		const auto* primitive = self->PrimitiveByIndex(i);
		const auto* mesh = primitive->mesh;
		if (mesh)
		{
			// Enumerate all triangles and create triaccels
			const auto* positions = mesh->Positions();
			const auto* faces = mesh->Faces();
			for (int j = 0; j < mesh->NumFaces() / 3; j++)
			{
				triAccels.push_back(TriAccel());
				triAccels.back().shapeIndex = j;
				triAccels.back().primIndex = i;
				unsigned int i1 = faces[3*j  ];
				unsigned int i2 = faces[3*j+1];
				unsigned int i3 = faces[3*j+2];
				triAccels.back().Load(
					Math::Vec3(positions[3*i1], positions[3*i1+1], positions[3*i1+2]),
					Math::Vec3(positions[3*i2], positions[3*i2+1], positions[3*i2+2]),
					Math::Vec3(positions[3*i3], positions[3*i3+1], positions[3*i3+2]));
			}
		}
	}
	
	return true;
}

bool NaiveScene::Impl::Intersect( Ray& ray, Intersection& isect )
{
	bool intersected = false;
	size_t minTriAccelIdx = 0;
	Math::Vec2 minB;

	for (size_t i = 0; i < triAccels.size(); i++)
	{
		Math::Float t;
		Math::Vec2 b;
		if (triAccels[i].Intersect(ray, ray.minT, ray.maxT, b[0], b[1], t))
		{
			ray.maxT = t;
			minTriAccelIdx = i;
			minB = b;
			intersected = true;
		}
	}

	if (intersected)
	{
		// Store required data for the intersection structure
		auto& triAccel = triAccels[minTriAccelIdx];

		// Primitive
		isect.primitiveIndex = triAccel.primIndex;
		isect.triangleIndex = triAccel.shapeIndex;
		isect.primitive = self->PrimitiveByIndex(triAccel.primIndex);

		const auto* mesh = isect.primitive->mesh;
		const auto* positions = mesh->Positions();
		const auto* normals = mesh->Normals();
		const auto* texcoords = mesh->TexCoords();
		const auto* faces = mesh->Faces();

		// Intersection point
		isect.p = ray.o + ray.d * ray.maxT;

		// Geometry normal
		int v1 = faces[3*triAccel.shapeIndex  ];
		int v2 = faces[3*triAccel.shapeIndex+1];
		int v3 = faces[3*triAccel.shapeIndex+2];
		auto& p1 = Math::Vec3(positions[3*v1], positions[3*v1+1], positions[3*v1+2]);
		auto& p2 = Math::Vec3(positions[3*v2], positions[3*v2+1], positions[3*v2+2]);
		auto& p3 = Math::Vec3(positions[3*v3], positions[3*v3+1], positions[3*v3+2]);
		isect.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));

		// Shading normal
		auto& n1 = Math::Vec3(normals[3*v1], normals[3*v1+1], normals[3*v1+2]);
		auto& n2 = Math::Vec3(normals[3*v2], normals[3*v2+1], normals[3*v2+2]);
		auto& n3 = Math::Vec3(normals[3*v3], normals[3*v3+1], normals[3*v3+2]);
		isect.sn = Math::Normalize(n1 * Math::Float(Math::Float(1) - minB[0] - minB[1]) + n2 * minB[0] + n3 * minB[1]);
		
		// Texture coordinates
		if (texcoords)
		{
			auto& uv1 = Math::Vec2(texcoords[2*v1], texcoords[2*v1+1]);
			auto& uv2 = Math::Vec2(texcoords[2*v2], texcoords[2*v2+1]);
			auto& uv3 = Math::Vec2(texcoords[2*v3], texcoords[2*v3+1]);
			isect.uv = uv1 * Math::Float(Math::Float(1) - minB[0] - minB[1]) + uv2 * minB[0] + uv3 * minB[1];
		}

		// Tangent vectors
		Math::OrthonormalBasis(isect.sn, isect.ss, isect.st);
	}

	return intersected;
}

// --------------------------------------------------------------------------------

NaiveScene::NaiveScene()
	: p(new Impl(this))
{

}

NaiveScene::~NaiveScene()
{
	NANON_SAFE_DELETE(p);
}

bool NaiveScene::Build()
{
	return p->Build();
}

bool NaiveScene::Intersect( Ray& ray, Intersection& isect ) const
{
	return p->Intersect(ray, isect);
}

std::string NaiveScene::Type() const
{
	return "naive";
}

NANON_NAMESPACE_END