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

#pragma once
#ifndef LIB_LIGHTMETRICA_BSDF_H
#define LIB_LIGHTMETRICA_BSDF_H

#include "generalizedbsdf.h"

LM_NAMESPACE_BEGIN

/*!
	BSDF.
	A base class for BSDF implementations.
*/
class LM_PUBLIC_API BSDF : public GeneralizedBSDF
{
public:

	BSDF(const std::string& id);
	virtual ~BSDF();

public:

	std::string Name() const { return "bsdf"; }

protected:

	/*!
		Compute correction factor for shading normal.
		See [Veach 1997] for details.
		\param transportDir Transport direction.
		\param geom Surface geometry.
		\param localWi #wi in local shading coordinates.
		\param localWo #wo in local shading coordinates.
		\param worldWi #wi in world coordinates.
		\param worldWo #wo in world coordinates.
	*/
	Math::Float ShadingNormalCorrectionFactor(
		const TransportDirection& transportDir,
		const SurfaceGeometry& geom,
		const Math::Vec3& localWi, const Math::Vec3& localWo,
		const Math::Vec3& worldWi, const Math::Vec3& worldWo) const;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_BSDF_H