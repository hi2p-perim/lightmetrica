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
#include <lightmetrica/bpt.fullpath.h>
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/transportdirection.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/assert.h>

LM_NAMESPACE_BEGIN

BPTFullPath::BPTFullPath( int s, int t, const BPTSubpath& lightSubpath, const BPTSubpath& eyeSubpath ) : s(s)
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