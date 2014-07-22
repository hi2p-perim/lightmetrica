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

#include "pch.plugin.h"
#include "simdsupport.h"
#include <lightmetrica/scene.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/primitives.h>
#include <lightmetrica/trianglemesh.h>
#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>

LM_NAMESPACE_BEGIN

#if LM_SSE2 && LM_SINGLE_PRECISION

/*!
	Embree accelerated scene.
	A scene accelerated with Embree, high-performance ray tracing kernels:
	http://embree.github.io/
*/
class EmbreeScene : public Scene
{
public:

	LM_COMPONENT_IMPL_DEF("plugin.embree");

public:

	EmbreeScene();
	~EmbreeScene();

public:

	virtual bool Configure( const ConfigNode& node ) { return true; }
	virtual boost::signals2::connection Connect_ReportBuildProgress( const std::function<void (double, bool ) >& func) { return signal_ReportBuildProgress.connect(func); }
	virtual bool Build();
	virtual bool Intersect( Ray& ray, Intersection& isect ) const;

public:

	static void EmbreeErrorHandler(const RTCError code, const char* str);

private:

	boost::signals2::signal<void (double, bool)> signal_ReportBuildProgress;

private:

	RTCScene rtcScene;													//!< Embree scene 
	std::unordered_map<unsigned int, int> rtcGeomIDToPrimitiveIDMap;	//!< Map between geometry IDs of Embree to primitive IDs

};

EmbreeScene::EmbreeScene()
{
	rtcInit(nullptr);
	rtcSetErrorFunction(EmbreeScene::EmbreeErrorHandler);
}

EmbreeScene::~EmbreeScene()
{
	rtcDeleteScene(rtcScene);
	rtcExit();
}

void EmbreeScene::EmbreeErrorHandler( const RTCError code, const char* str )
{
	std::string error = "";
	switch (code)
	{
		case RTC_UNKNOWN_ERROR:		{ error = "RTC_UNKNOWN_ERROR";		break; }
		case RTC_INVALID_ARGUMENT:	{ error = "RTC_INVALID_ARGUMENT";	break; }
		case RTC_INVALID_OPERATION:	{ error = "RTC_INVALID_OPERATION";	break; }
		case RTC_OUT_OF_MEMORY:		{ error = "RTC_OUT_OF_MEMORY";		break; }
		case RTC_UNSUPPORTED_CPU:	{ error = "RTC_UNSUPPORTED_CPU";	break; }
		default:					{ error = "Invalid error code";		break; }
	}
	LM_LOG_ERROR("Embree error : " + error);
}

bool EmbreeScene::Build()
{
	signal_ReportBuildProgress(0, false);

	// Create scene
	rtcScene = rtcNewScene(RTC_SCENE_STATIC | RTC_SCENE_INCOHERENT, RTC_INTERSECT1);

	// Add primitives to the scene
	int numPrimitives = primitives->NumPrimitives();
	for (int i = 0; i < numPrimitives; i++)
	{
		const auto* primitive = primitives->PrimitiveByIndex(i);
		const auto* mesh = primitive->mesh;
		if (mesh)
		{
			// Create a triangle mesh
			unsigned int geomId = rtcNewTriangleMesh(rtcScene, RTC_GEOMETRY_STATIC, mesh->NumFaces() / 3, mesh->NumFaces());
			rtcGeomIDToPrimitiveIDMap[geomId] = i;

			// Copy vertices & faces
			auto* mappedPositions = reinterpret_cast<Math::Float*>(rtcMapBuffer(rtcScene, geomId, RTC_VERTEX_BUFFER));
			auto* mappedFaces     = reinterpret_cast<int*>(rtcMapBuffer(rtcScene, geomId, RTC_INDEX_BUFFER));
			const auto* positions = mesh->Positions();
			const auto* faces = mesh->Faces();
			for (int j = 0; j < mesh->NumFaces() / 3; j++)
			{
				// Transform positions
				unsigned int i1 = faces[3*j  ];
				unsigned int i2 = faces[3*j+1];
				unsigned int i3 = faces[3*j+2];
				Math::Vec3 p1(primitive->transform * Math::Vec4(positions[3*i1], positions[3*i1+1], positions[3*i1+2], Math::Float(1)));
				Math::Vec3 p2(primitive->transform * Math::Vec4(positions[3*i2], positions[3*i2+1], positions[3*i2+2], Math::Float(1)));
				Math::Vec3 p3(primitive->transform * Math::Vec4(positions[3*i3], positions[3*i3+1], positions[3*i3+2], Math::Float(1)));
				
				// Store into mapped buffers
				int mi1 = 3*j;
				int mi2 = 3*j+1;
				int mi3 = 3*j+2;
				mappedFaces[mi1] = mi1;
				mappedFaces[mi2] = mi2;
				mappedFaces[mi3] = mi3;
				for (int k = 0; k < 3; k++)
				{
					mappedPositions[4*mi1+k] = p1[k];
					mappedPositions[4*mi2+k] = p2[k];
					mappedPositions[4*mi3+k] = p3[k];
				}
			}
			rtcUnmapBuffer(rtcScene, geomId, RTC_VERTEX_BUFFER); 
			rtcUnmapBuffer(rtcScene, geomId, RTC_INDEX_BUFFER);
		}

		signal_ReportBuildProgress(static_cast<double>(i) / numPrimitives, false);
	}

	rtcCommit (rtcScene);
	signal_ReportBuildProgress(1, true);

	return true;
}

bool EmbreeScene::Intersect( Ray& ray, Intersection& isect ) const
{
	// Convert #ray to RTCRay
	RTCRay rtcRay;
	rtcRay.org[0] = ray.o[0];
	rtcRay.org[1] = ray.o[1];
	rtcRay.org[2] = ray.o[2];
	rtcRay.dir[0] = ray.d[0];
	rtcRay.dir[1] = ray.d[1];
	rtcRay.dir[2] = ray.d[2];
	rtcRay.tnear = ray.minT;
	rtcRay.tfar = ray.maxT;
	rtcRay.geomID = RTC_INVALID_GEOMETRY_ID;
	rtcRay.primID = RTC_INVALID_GEOMETRY_ID;
	rtcRay.instID = RTC_INVALID_GEOMETRY_ID;
	rtcRay.mask = 0xFFFFFFFF;
	rtcRay.time = 0;

	// Intersection query
	rtcIntersect(rtcScene, rtcRay);
	if (rtcRay.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// No hits
		return false;
	}

	// Store information into #isect
	isect.primitiveIndex = rtcGeomIDToPrimitiveIDMap.at(rtcRay.geomID);
	isect.triangleIndex = rtcRay.primID;
	isect.primitive = primitives->PrimitiveByIndex(isect.primitiveIndex);

	const auto* mesh = isect.primitive->mesh;
	const auto* positions = mesh->Positions();
	const auto* normals = mesh->Normals();
	const auto* texcoords = mesh->TexCoords();
	const auto* faces = mesh->Faces();

	// Intersection point
	isect.geom.p = ray.o + ray.d * rtcRay.tfar;

	// Geometry normal
	int v1 = faces[3*isect.triangleIndex  ];
	int v2 = faces[3*isect.triangleIndex+1];
	int v3 = faces[3*isect.triangleIndex+2];
	Math::Vec3 p1(isect.primitive->transform * Math::Vec4(positions[3*v1], positions[3*v1+1], positions[3*v1+2], Math::Float(1)));
	Math::Vec3 p2(isect.primitive->transform * Math::Vec4(positions[3*v2], positions[3*v2+1], positions[3*v2+2], Math::Float(1)));
	Math::Vec3 p3(isect.primitive->transform * Math::Vec4(positions[3*v3], positions[3*v3+1], positions[3*v3+2], Math::Float(1)));
	isect.geom.gn = Math::Normalize(Math::Cross(p2 - p1, p3 - p1));

	// Shading normal
	Math::Vec3 n1(isect.primitive->normalTransform * Math::Vec3(normals[3*v1], normals[3*v1+1], normals[3*v1+2]));
	Math::Vec3 n2(isect.primitive->normalTransform * Math::Vec3(normals[3*v2], normals[3*v2+1], normals[3*v2+2]));
	Math::Vec3 n3(isect.primitive->normalTransform * Math::Vec3(normals[3*v3], normals[3*v3+1], normals[3*v3+2]));
	isect.geom.sn = Math::Normalize(n1 * (Math::Float(1) - rtcRay.u - rtcRay.v) + n2 * rtcRay.u + n3 * rtcRay.v);

	// Texture coordinates
	if (texcoords)
	{
		Math::Vec2 uv1(texcoords[2*v1], texcoords[2*v1+1]);
		Math::Vec2 uv2(texcoords[2*v2], texcoords[2*v2+1]);
		Math::Vec2 uv3(texcoords[2*v3], texcoords[2*v3+1]);
		isect.geom.uv = uv1 * Math::Float(Math::Float(1) - rtcRay.u - rtcRay.v) + uv2 * rtcRay.u + uv3 * rtcRay.v;
	}

	// Scene surface is not degenerated
	isect.geom.degenerated = false;

	// Compute tangent space
	isect.geom.ComputeTangentSpace();

	return true;
}

LM_COMPONENT_REGISTER_PLUGIN_IMPL(EmbreeScene, Scene);

#endif

LM_NAMESPACE_END