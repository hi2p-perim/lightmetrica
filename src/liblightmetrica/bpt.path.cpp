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
#include <lightmetrica/bpt.path.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/emitter.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/assert.h>

LM_NAMESPACE_BEGIN

BPTPathVertex::BPTPathVertex()
	: type(BPTPathVertexType::None)
	, emitter(nullptr)
	, bsdf(nullptr)
	, areaLight(nullptr)
	, areaCamera(nullptr)
{

}

void BPTPathVertex::DebugPrint() const
{
	LM_LOG_DEBUG("Type : " +
		std::string(
		type == BPTPathVertexType::EndPoint ? "EndPoint" :
		type == BPTPathVertexType::IntermediatePoint ? "IntermediatePoint" : "None"));

	if (type == BPTPathVertexType::None)
	{
		return;
	}

	LM_LOG_DEBUG("Transport direction : " + std::string(transportDir == TransportDirection::EL ? "EL" : "LE"));

	{
		LM_LOG_DEBUG("Surface geometry");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Degenerated : " + std::string(geom.degenerated ? "True" : "False"));
		LM_LOG_DEBUG(boost::str(boost::format("Position : (%f, %f, %f)") % geom.p.x % geom.p.y % geom.p.z));
		if (!geom.degenerated)
		{
			LM_LOG_DEBUG(boost::str(boost::format("Geometry normal : (%f, %f, %f)") % geom.gn.x % geom.gn.y % geom.gn.z));
			LM_LOG_DEBUG(boost::str(boost::format("Shading normal : (%f, %f, %f)") % geom.sn.x % geom.sn.y % geom.sn.z));
		}
	}

	static const std::string ProbabilityMeasureNames[] =
	{
		"None",
		"SolidAngle",
		"ProjectedSolidAngle",
		"Area",
		"Discrete"
	};

	if (type == BPTPathVertexType::EndPoint)
	{
		LM_LOG_DEBUG("Emitter type : " + emitter->Name() + " (" + emitter->Type() + ")");
		{
			LM_LOG_DEBUG("PDF (positional component)");
			LM_LOG_INDENTER();
			LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfP.measure)]);
			LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfP.v));
		}
	}
	else if (type == BPTPathVertexType::IntermediatePoint)
	{
		LM_LOG_DEBUG("Generalized BSDF type : " + bsdf->Name() + " (" + bsdf->Type() + ")");
		{
			LM_LOG_DEBUG("PDF (directional component, E->L)");
			LM_LOG_INDENTER();
			LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfD[TransportDirection::EL].measure)]);
			LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfD[TransportDirection::EL].v));
		}
		{
			LM_LOG_DEBUG("PDF (directional component, E->L)");
			LM_LOG_INDENTER();
			LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfD[TransportDirection::LE].measure)]);
			LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfD[TransportDirection::LE].v));
		}
	}
}

// --------------------------------------------------------------------------------

void BPTPath::Clear()
{
	vertices.clear();
}

void BPTPath::Add( BPTPathVertex* vertex )
{
	vertices.push_back(vertex);
}

void BPTPath::Release( BPTPathVertexPool& pool )
{
	for (auto* vertex : vertices)
	{
		pool.Release(vertex);
	}

	vertices.clear();
}

void BPTPath::DebugPrint()
{
	for (size_t i = 0; i < vertices.size(); i++)
	{
		LM_LOG_DEBUG("Vertex #" + std::to_string(i));
		LM_LOG_INDENTER();
		vertices[i]->DebugPrint();
	}
}

// --------------------------------------------------------------------------------

BPTFullPath::BPTFullPath( int s, int t, const BPTPath& lightSubpath, const BPTPath& eyeSubpath ) : s(s)
	, t(t)
	, lightSubpath(lightSubpath)
	, eyeSubpath(eyeSubpath)
{
	LM_ASSERT(s > 0 || t > 0);
	LM_ASSERT(s + t >= 2);

	// Compute #pdfDL and #pdfDE
	if (s == 0 && t > 0)
	{
		// Compute #pdfDE[LE]
		auto* z = eyeSubpath.vertices[t-1];
		if (z->areaLight)
		{
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.type = GeneralizedBSDFType::LightDirection;
			bsdfEQ.wo = z->wi;
			pdfDE[TransportDirection::LE] = z->areaLight->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}
	}
	else if (s > 0 && t == 0)
	{
		// Compute #pdfDL[EL]
		auto* y = lightSubpath.vertices[s-1];
		if (y->areaCamera)
		{
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
			bsdfEQ.wo = y->wi;
			pdfDL[TransportDirection::EL] = y->areaCamera->EvaluateDirectionPDF(bsdfEQ, y->geom);
		}
	}
	else if (s > 0 && t > 0)
	{
		auto* y = lightSubpath.vertices[s-1];
		auto* z = eyeSubpath.vertices[t-1];

		GeneralizedBSDFEvaluateQuery bsdfEQ;
		bsdfEQ.type = GeneralizedBSDFType::All;

		auto yz = Math::Normalize(z->geom.p - y->geom.p);
		auto zy = -yz;

		// Compute #pdfDL[EL]
		if (s > 1)
		{
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = yz;
			bsdfEQ.wo = y->wi;
			pdfDL[TransportDirection::EL] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);
		}

		// Compute #pdfDL[LE]
		bsdfEQ.transportDir = TransportDirection::LE;
		bsdfEQ.wi = y->wi;
		bsdfEQ.wo = yz;
		pdfDL[TransportDirection::LE] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);

		// Compute #pdfDE[LE]
		if (t > 1)
		{
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = zy;
			bsdfEQ.wo = z->wi;
			pdfDE[TransportDirection::LE] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}

		// Compute #pdfDE[EL]
		bsdfEQ.transportDir = TransportDirection::EL;
		bsdfEQ.wi = z->wi;
		bsdfEQ.wo = zy;
		pdfDE[TransportDirection::EL] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
	}
}

LM_NAMESPACE_END