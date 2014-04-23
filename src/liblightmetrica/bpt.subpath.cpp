
#include "pch.h"
#include <lightmetrica/bpt.subpath.h>
#include <lightmetrica/bpt.pool.h>
#include <lightmetrica/bpt.config.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/generalizedbsdf.h>
#include <lightmetrica/emitter.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/light.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/random.h>
#include <lightmetrica/assert.h>
#include <lightmetrica/ray.h>
#include <lightmetrica/intersection.h>
#include <lightmetrica/primitive.h>

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

void BPTSubpath::Release( BPTPathVertexPool& pool )
{
	for (auto* vertex : vertices)
	{
		pool.Release(vertex);
	}

	vertices.clear();
}

void BPTSubpath::DebugPrint() const
{
	for (size_t i = 0; i < vertices.size(); i++)
	{
		LM_LOG_DEBUG("Vertex #" + std::to_string(i));
		LM_LOG_INDENTER();
		vertices[i]->DebugPrint();
	}
}

void BPTSubpath::SampleSubpath( const BPTConfig& config, const Scene& scene, Random& rng, BPTPathVertexPool& pool, TransportDirection transportDir )
{
	LM_ASSERT(vertices.empty());

	BPTPathVertex* v;

	// Initial vertex
	v = pool.Construct();
	v->type = BPTPathVertexType::EndPoint;
	v->transportDir = transportDir;

	// Positional component
	if (transportDir == TransportDirection::EL)
	{
		// EyePosition
		v->emitter = scene.MainCamera();
		v->emitter->SamplePosition(rng.NextVec2(), v->geom, v->pdfP);
	}
	else
	{
		// LightPosition
		auto lightSampleP = rng.NextVec2();
		Math::PDFEval lightSelectionPdf;
		v->emitter = scene.SampleLightSelection(lightSampleP, lightSelectionPdf);
		v->emitter->SamplePosition(lightSampleP, v->geom, v->pdfP);
		v->pdfP.v *= lightSelectionPdf.v;
	}

	// Directional component
	v->bsdf = v->emitter;

	GeneralizedBSDFSampleQuery bsdfSQE;
	bsdfSQE.sample = rng.NextVec2();
	bsdfSQE.transportDir = transportDir;
	bsdfSQE.type = GeneralizedBSDFType::AllEmitter;

	GeneralizedBSDFSampleResult bsdfSRE;
	v->bsdf->SampleDirection(bsdfSQE, v->geom, bsdfSRE);
	v->pdfD[transportDir] = bsdfSRE.pdf;
	v->pdfD[1-transportDir] = Math::PDFEval();
	v->wo = bsdfSRE.wo;

	// # of vertices is always greater than 1
	v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);

	vertices.push_back(v);

	// --------------------------------------------------------------------------------

	int depth = 1;
	while (true)
	{
		// Previous vertex
		auto* pv = vertices.back();

		// Create ray
		Ray ray;
		ray.d = pv->wo;
		ray.o = pv->geom.p;
		ray.minT = Math::Constants::Eps();
		ray.maxT = Math::Constants::Inf();

		// Check intersection
		Intersection isect;
		if (!scene.Intersect(ray, isect))
		{
			break;
		}

		// --------------------------------------------------------------------------------

		// Create path vertex
		v = pool.Construct();
		v->type = BPTPathVertexType::IntermediatePoint;
		v->transportDir = transportDir;
		v->bsdf = isect.primitive->bsdf;
		v->geom = isect.geom;
		v->wi = -pv->wo;

		// Area light or camera
		v->areaLight = isect.primitive->light;
		v->areaCamera = isect.primitive->camera;

		// Area light or camera should not be associated with the same surfaces
		LM_ASSERT(v->areaLight == nullptr || v->areaCamera == nullptr);
		if (v->areaLight)
		{
			v->emitter = v->areaLight;
		}
		if (v->areaCamera)
		{
			v->emitter = v->areaCamera;
		}
		if (v->emitter)
		{
			// Calculate #pdfP for intersected emitter
			v->pdfP = v->emitter->EvaluatePositionPDF(v->geom);
		}
		else
		{
			v->pdfP = Math::PDFEval(Math::Float(0), Math::ProbabilityMeasure::Area);
		}

		// --------------------------------------------------------------------------------

		// Apply RR
		int rrDepthT = config.rrDepth;
#if LM_ENABLE_BPT_EXPERIMENTAL
		if (config.enableExperimentalMode)
		{
			// At least #maxSubpathNumVertices vertices are sampled in the experimental mode
			rrDepthT = Math::Max(rrDepthT, config.maxSubpathNumVertices);
		}
#endif

		if (++depth >= rrDepthT)
		{
			// TODO : Replace with the more efficient one
			auto p = Math::Float(0.5);
			if (rng.Next() > p)
			{
				vertices.push_back(v);
				break;
			}

			// RR probability
			v->pdfRR = Math::PDFEval(p, Math::ProbabilityMeasure::Discrete);
		}
		else
		{
			v->pdfRR = Math::PDFEval(Math::Float(1), Math::ProbabilityMeasure::Discrete);
		}

		// --------------------------------------------------------------------------------

		// Sample generalized BSDF
		GeneralizedBSDFSampleQuery bsdfSQ;
		bsdfSQ.sample = rng.NextVec2();
		bsdfSQ.transportDir = transportDir;
		bsdfSQ.type = GeneralizedBSDFType::All;
		bsdfSQ.wi = -pv->wo;

		GeneralizedBSDFSampleResult bsdfSR;
		if (!v->bsdf->SampleDirection(bsdfSQ, v->geom, bsdfSR))
		{
			vertices.push_back(v);
			break;
		}

		v->wo = bsdfSR.wo;
		v->pdfD[transportDir] = bsdfSR.pdf;

		// Evaluate PDF in the opposite transport direction
		// TODO : Handle specular case
		GeneralizedBSDFEvaluateQuery bsdfEQ;
		bsdfEQ.type = bsdfSR.sampledType;
		bsdfEQ.transportDir = TransportDirection(1 - transportDir);
		bsdfEQ.wi = bsdfSR.wo;
		bsdfEQ.wo = bsdfSQ.wi;
		v->pdfD[1-transportDir] = v->bsdf->EvaluateDirectionPDF(bsdfEQ, v->geom);

		vertices.push_back(v);
	}
}

LM_NAMESPACE_END