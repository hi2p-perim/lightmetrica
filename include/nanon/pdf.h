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

#pragma once
#ifndef __LIB_NANON_PDF_H__
#define __LIB_NANON_PDF_H__

#include "common.h"
#include "math.types.h"

NANON_NAMESPACE_BEGIN

/*!
*/
enum class ProbabilityMeasure
{
	/*!
		Solid angle measure.
		P_\sigma(x\to y).
	*/
	SolidAngle,
	
	/*!
		Area measure.
		P_A(x).
	*/
	Area,

};

/*!
*/
struct PDF
{

	PDF() {}
	PDF(const Math::Float& v, ProbabilityMeasure measure)
		: v(v)
		, measure(measure)
	{}

	Math::Float v;					//!< Value of the PDF.
	ProbabilityMeasure measure;		//!< Probability measure that the PDF is defined.

};

NANON_NAMESPACE_END

#endif // __LIB_NANON_PDF_H__