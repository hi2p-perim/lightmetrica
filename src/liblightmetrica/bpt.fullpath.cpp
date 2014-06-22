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
#include <lightmetrica/scene.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/renderutils.h>

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
		auto* z		= eyeSubpath.vertices[t-1];
		auto* zPrev	= eyeSubpath.vertices[t-2];		// Always non-null because t >= 2
		if (z->areaLight)
		{
			if (!zPrev->geom.degenerated)
			{
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::LE;
				bsdfEQ.type = GeneralizedBSDFType::LightDirection;
				bsdfEQ.wo = z->wi;
				pdfDE[TransportDirection::LE] = z->areaLight->EvaluateDirectionPDF(bsdfEQ, z->geom);
			}
			else
			{
				pdfDE[TransportDirection::LE] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
		}
		else
		{
			// This must be set
			pdfDE[TransportDirection::LE] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
	}
	else if (s > 0 && t == 0)
	{
		// Compute #pdfDL[EL]
		auto* y		= lightSubpath.vertices[s-1];
		auto* yPrev	= lightSubpath.vertices[s-2];
		if (y->areaCamera)
		{
			if (!yPrev->geom.degenerated)
			{
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.type = GeneralizedBSDFType::EyeDirection;
				bsdfEQ.wo = y->wi;
				pdfDL[TransportDirection::EL] = y->areaCamera->EvaluateDirectionPDF(bsdfEQ, y->geom);
			}
			else
			{
				pdfDL[TransportDirection::EL] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
		}
		else
		{
			// This must be set
			pdfDL[TransportDirection::EL] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
	}
	else if (s > 0 && t > 0)
	{
		auto* y = lightSubpath.vertices[s-1];
		auto* yPrev = s > 1 ? lightSubpath.vertices[s-2] : nullptr;
		auto* z = eyeSubpath.vertices[t-1];
		auto* zPrev = t > 1 ? eyeSubpath.vertices[t-2] : nullptr;

		GeneralizedBSDFEvaluateQuery bsdfEQ;
		bsdfEQ.type = GeneralizedBSDFType::All;

		auto yz = Math::Normalize(z->geom.p - y->geom.p);
		auto zy = -yz;

		// Compute #pdfDL[EL]
		if (yPrev != nullptr)
		{
			if (!yPrev->geom.degenerated)
			{
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.wi = yz;
				bsdfEQ.wo = y->wi;
				pdfDL[TransportDirection::EL] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);
			}
			else
			{
				// Handle degenerated vertex (specular surfaces or point lights)
				pdfDL[TransportDirection::EL] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
		}
		else
		{
			// Invalid (not used)
			pdfDL[TransportDirection::EL] = Math::PDFEval();
		}

		// Compute #pdfDL[LE]
		if (!z->geom.degenerated)
		{
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = y->wi;
			bsdfEQ.wo = yz;
			pdfDL[TransportDirection::LE] = y->bsdf->EvaluateDirectionPDF(bsdfEQ, y->geom);
		}
		else
		{
			// z is degenerated
			pdfDL[TransportDirection::LE] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
		
		// Compute #pdfDE[LE]
		if (zPrev != nullptr)
		{
			if (!zPrev->geom.degenerated)
			{
				bsdfEQ.transportDir = TransportDirection::LE;
				bsdfEQ.wi = zy;
				bsdfEQ.wo = z->wi;
				pdfDE[TransportDirection::LE] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
			}
			else
			{
				// Handle degenerated vertex (specular surfaces or perspective camera)
				pdfDE[TransportDirection::LE] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
			}
		}
		else
		{
			// Invalid (not used)
			pdfDE[TransportDirection::LE] = Math::PDFEval();
		}

		// Compute #pdfDE[EL]
		if (!y->geom.degenerated)
		{
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = z->wi;
			bsdfEQ.wo = zy;
			pdfDE[TransportDirection::EL] = z->bsdf->EvaluateDirectionPDF(bsdfEQ, z->geom);
		}
		else
		{
			// y is degenerated
			pdfDE[TransportDirection::EL] = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::ProjectedSolidAngle);
		}
	}
}

Math::Vec3 BPTFullPath::EvaluateUnweightContribution( const Scene& scene, Math::Vec2& rasterPosition ) const
{
	// Evaluate \alpha^L_s
	auto alphaL = lightSubpath.EvaluateSubpathAlpha(s, rasterPosition);
	if (Math::IsZero(alphaL))
	{
		return Math::Vec3();
	}

	// Evaluate \alpha^E_t
	auto alphaE = eyeSubpath.EvaluateSubpathAlpha(t, rasterPosition);
	if (Math::IsZero(alphaE))
	{
		return Math::Vec3();
	}
	
	// --------------------------------------------------------------------------------

	// Evaluate c_{s,t}
	Math::Vec3 cst;
	
	if (s == 0 && t > 0)
	{
		// z_{t-1} is area light
		auto* v = eyeSubpath.vertices[t-1];
		if (v->areaLight)
		{
			// Camera emitter cannot be an light
			LM_ASSERT(t >= 1);

			// Evaluate Le^0(z_{t-1})
			cst = v->areaLight->EvaluatePosition(v->geom);

			// Evaluate Le^1(z_{t-1}\to z_{t-2})
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wo = v->wi;
			cst *= v->areaLight->EvaluateDirection(bsdfEQ, v->geom);
		}
	}
	else if (s > 0 && t == 0)
	{
		// y_{s-1} is area camera
		auto* v = lightSubpath.vertices[s-1];
		if (v->areaCamera)
		{
			// Light emitter cannot be an camera
			LM_ASSERT(s >= 1);

			// Raster position
			if (v->areaCamera->RayToRasterPosition(v->geom.p, v->wi, rasterPosition))
			{
				// Evaluate We^0(y_{s-1})
				cst = v->areaCamera->EvaluatePosition(v->geom);

				// Evaluate We^1(y_{s-1}\to y_{s-2})
				GeneralizedBSDFEvaluateQuery bsdfEQ;
				bsdfEQ.type = GeneralizedBSDFType::AllEmitter;
				bsdfEQ.transportDir = TransportDirection::EL;
				bsdfEQ.wo = v->wi;
				cst *= v->areaCamera->EvaluateDirection(bsdfEQ, v->geom);
			}
		}
	}
	else if (s > 0 && t > 0)
	{
		auto* vL = lightSubpath.vertices[s-1];
		auto* vE = eyeSubpath.vertices[t-1];

		// Both #vL and #vE must not be directionally degenerated
		// which avoids unnecessary intersection query and BSDF evaluation
		if (vL->bsdf->Degenerated() || vE->bsdf->Degenerated())
		{
			return Math::Vec3();
		}

		// Check connectivity between #vL->geom.p and #vE->geom.p
		Ray shadowRay;
		auto pLpE = vE->geom.p - vL->geom.p;
		auto pLpE_Length = Math::Length(pLpE);
		shadowRay.d = pLpE / pLpE_Length;
		shadowRay.o = vL->geom.p;
		shadowRay.minT = Math::Constants::Eps();
		shadowRay.maxT = pLpE_Length * (Math::Float(1) - Math::Constants::Eps());

		// Update raster position if #t = 1
		bool visible = true;
		if (t == 1)
		{
			visible = scene.MainCamera()->RayToRasterPosition(vE->geom.p, -shadowRay.d, rasterPosition);
		}

		Intersection shadowIsect;
		if (visible && !scene.Intersect(shadowRay, shadowIsect))
		{			
			GeneralizedBSDFEvaluateQuery bsdfEQ;
			bsdfEQ.type = GeneralizedBSDFType::All;

			// fsL
			bsdfEQ.transportDir = TransportDirection::LE;
			bsdfEQ.wi = vL->wi;
			bsdfEQ.wo = shadowRay.d;
			auto fsL = vL->bsdf->EvaluateDirection(bsdfEQ, vL->geom);

			// fsE
			bsdfEQ.transportDir = TransportDirection::EL;
			bsdfEQ.wi = vE->wi;
			bsdfEQ.wo = -shadowRay.d;
			auto fsE = vE->bsdf->EvaluateDirection(bsdfEQ, vE->geom);

			// Geometry term
			auto G = RenderUtils::GeneralizedGeometryTerm(vL->geom, vE->geom);

			cst = fsL * G * fsE;
		}
	}

	if (Math::IsZero(cst))
	{
		return Math::Vec3();
	}

	// --------------------------------------------------------------------------------

	// Evaluate contribution C^*_{s,t} = \alpha^L_s * c_{s,t} * \alpha^E_t
	return alphaL * cst * alphaE;
}

Math::Float BPTFullPath::EvaluateFullpathPDF( int i ) const
{
	int n = s + t;
	if (0 < i && i < n)
	{
		// If at least one of generalized BSDFs associated with connection vertices is degenerated,
		// the probability is zero because this full path cannot be sampled with p_i.
		const auto* xL = FullPathVertex(i-1);
		const auto* xE = FullPathVertex(i);
		if (xL->bsdf->Degenerated() || xE->bsdf->Degenerated())
		{
			return Math::Float(0);
		}
	}

	Math::Float fullpathPdf(1);

	if (i > 0)
	{
		// Evaluate p_A(x_0)
		const auto* x0 = FullPathVertex(0);
		LM_ASSERT(x0->pdfP.measure == Math::ProbabilityMeasure::Area);
		fullpathPdf *= x0->pdfP.v;

		// Evaluate p_{\sigma^\bot}(x_j\to x_{j+1}) * G(x_j\leftrightarrow x_{j+1}) (j = 0 to i-2)
		for (int j = 0; j <= i-2; j++)
		{
			const auto* xj		= FullPathVertex(j);
			const auto* xjNext	= FullPathVertex(j+1);
			auto xjPdfDLE		= FullPathVertexDirectionPDF(j, TransportDirection::LE);
			LM_ASSERT(xjPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
			fullpathPdf *=
				xjPdfDLE.v *
				RenderUtils::GeneralizedGeometryTerm(xj->geom, xjNext->geom);
		}
	}

	if (i < n)
	{
		// Evaluate p_A(x_{n-1})
		const auto* xnPrev = FullPathVertex(n-1);
		LM_ASSERT(xnPrev->pdfP.measure == Math::ProbabilityMeasure::Area);
		fullpathPdf *= xnPrev->pdfP.v;
		
		// Evaluate p_{\sigma^\bot}(x_j\to x_{j-1}) * G(x_j\leftrightarrow x_{j-1}) (j = n-1 downto i+1)
		for (int j = n-1; j >= i+1; j--)
		{
			const auto* xj		= FullPathVertex(j);
			const auto* xjPrev	= FullPathVertex(j-1);
			auto xjPdfDEL		= FullPathVertexDirectionPDF(j, TransportDirection::EL);
			LM_ASSERT(xjPdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
			fullpathPdf *=
				xjPdfDEL.v *
				RenderUtils::GeneralizedGeometryTerm(xj->geom, xjPrev->geom);
		}
	}

	return fullpathPdf;
}

Math::Float BPTFullPath::EvaluateFullpathPDFRatio( int i ) const
{
	if (i == 0)
	{
		// p_1 / p_0 =
		//    p_A(x_0) /
		//    p_{\sigma^\bot}(x_1\to x_0)G(x_1\leftrightarrow x_0)
		const auto* x0	= FullPathVertex(0);
		const auto* x1	= FullPathVertex(1);
		auto x1PdfDEL	= FullPathVertexDirectionPDF(1, TransportDirection::EL);

		LM_ASSERT(x0->pdfP.measure == Math::ProbabilityMeasure::Area);
		LM_ASSERT(x1PdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

		auto denom = x1PdfDEL.v * RenderUtils::GeneralizedGeometryTerm(x0->geom, x1->geom);
		if (Math::Abs(denom) < Math::Constants::Eps())
		{
			return Math::Float(0);
		}

		return x0->pdfP.v / denom;
	}
	
	int n = s + t;
	if (i == n-1)
	{
		// p_n / p_{n-1} =
		//     p_{\sigma^\bot}(x_{n-2}\to x_{n-1})G(x_{n-2}\leftrightarrow x_{n-1}) /
		//     p_A(x_{n-1})
		const auto* xnPrev	= FullPathVertex(n-1);
		const auto* xnPrev2	= FullPathVertex(n-2);
		auto xnPrev2PdfDLE	= FullPathVertexDirectionPDF(n-2, TransportDirection::LE);

		LM_ASSERT(xnPrev->pdfP.measure == Math::ProbabilityMeasure::Area);
		LM_ASSERT(xnPrev2PdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

		auto denom = xnPrev->pdfP.v;
		if (Math::Abs(denom) < Math::Constants::Eps())
		{
			return Math::Float(0);
		}

		return xnPrev2PdfDLE.v * RenderUtils::GeneralizedGeometryTerm(xnPrev2->geom, xnPrev->geom) / denom;
	}

	// p_{i+1} / p_i =
	//     p_{\sigma^\bot}(x_{i-1}\to x_i)G(x_{i-1}\leftrightarrow x_i) /
	//     p_{\sigma^\bot}(x_{i+1}\to x_i)G(x_{i+1}\leftrightarrow x_i)
	const auto* xi		= FullPathVertex(i);
	const auto* xiNext	= FullPathVertex(i+1);
	const auto* xiPrev	= FullPathVertex(i-1);
	auto xiPrevPdfDLE	= FullPathVertexDirectionPDF(i-1, TransportDirection::LE);
	auto xiNextPdfDEL	= FullPathVertexDirectionPDF(i+1, TransportDirection::EL);

	LM_ASSERT(xiPrevPdfDLE.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);
	LM_ASSERT(xiNextPdfDEL.measure == Math::ProbabilityMeasure::ProjectedSolidAngle);

	auto denom = xiNextPdfDEL.v * RenderUtils::GeneralizedGeometryTerm(xiNext->geom, xi->geom);
	if (Math::Abs(denom) < Math::Constants::Eps())
	{
		return Math::Float(0);
	}

	return xiPrevPdfDLE.v * RenderUtils::GeneralizedGeometryTerm(xiPrev->geom, xi->geom) / denom;
}

const BPTPathVertex* BPTFullPath::FullPathVertex( int i ) const
{
	LM_ASSERT(0 <= i && i < s + t);
	return i < s
		? lightSubpath.vertices[i]
		: eyeSubpath.vertices[t-1-(i-s)];
}

Math::PDFEval BPTFullPath::FullPathVertexDirectionPDF( int i, TransportDirection transportDir ) const
{
	LM_ASSERT(0 <= i && i < s + t);
	return
		i == s - 1	? pdfDL[transportDir] :
		i == s		? pdfDE[transportDir]
					: FullPathVertex(i)->pdfD[transportDir];
}

void BPTFullPath::DebugPrint() const
{
	{
		LM_LOG_DEBUG("Connecting # of vertices");
		LM_LOG_INDENTER();
		LM_LOG_DEBUG("Light subpath : " + std::to_string(s));
		LM_LOG_DEBUG("  Eye subpath : " + std::to_string(t));
	}

	static const std::string ProbabilityMeasureNames[] =
	{
		"None",
		"SolidAngle",
		"ProjectedSolidAngle",
		"Area",
		"Discrete"
	};

	{
		LM_LOG_DEBUG("Directional PDF evaluations on connecting vertices");
		LM_LOG_INDENTER();
		{
			LM_LOG_DEBUG("Connecting vertices near L");
			LM_LOG_INDENTER();
			{
				LM_LOG_DEBUG("E->L");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfDL[TransportDirection::EL].measure)]);
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfDL[TransportDirection::EL].v));
			}
			{
				LM_LOG_DEBUG("L->E");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfDL[TransportDirection::LE].measure)]);
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfDL[TransportDirection::LE].v));
			}
		}
		{
			LM_LOG_DEBUG("Connecting vertices near E");
			LM_LOG_INDENTER();
			{
				LM_LOG_DEBUG("E->L");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfDE[TransportDirection::EL].measure)]);
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfDE[TransportDirection::EL].v));
			}
			{
				LM_LOG_DEBUG("L->E");
				LM_LOG_INDENTER();
				LM_LOG_DEBUG("Measure : " + ProbabilityMeasureNames[static_cast<int>(pdfDE[TransportDirection::LE].measure)]);
				LM_LOG_DEBUG(boost::str(boost::format("Eval : %f") % pdfDE[TransportDirection::LE].v));
			}
		}
	}

	{
		LM_LOG_DEBUG("Light sub-path");
		LM_LOG_INDENTER();
		lightSubpath.DebugPrint(s);
	}

	{
		LM_LOG_DEBUG("Eye sub-path");
		LM_LOG_INDENTER();
		eyeSubpath.DebugPrint(t);
	}
}

LM_NAMESPACE_END