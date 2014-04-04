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
#ifndef LIB_LIGHTMETRICA_MATH_PDF_H
#define LIB_LIGHTMETRICA_MATH_PDF_H

#include "math.common.h"

LM_NAMESPACE_BEGIN
LM_MATH_NAMESPACE_BEGIN

/*!
	Probability measure.
	Types of the probability measures.
*/
enum class ProbabilityMeasure
{

	/*!
		Invalid measure.
	*/
	None,

	/*!
		Solid angle measure.
		P_\sigma(x\to y).
	*/
	SolidAngle,
	
	/*!
		Projected solid angle measure.
		P_{\sigma^\bot}(x\to y). See [Veach 1997] for details.
	*/
	ProjectedSolidAngle,

	/*!
		Area measure.
		P_A(x).
	*/
	Area,

	/*!
		Discrete measure.
		Constructed from the measurable set (\Omega, 2^\Omega),
		where
			\Omega : Countable set,
			2^\Omega : Power set of \Omega
		Define P : 2^\Omega \to \mathbb{R}
		where
			P(A) = \sum_{\omega\in A} p(\omega), A\in 2^\Omega
			p_\omega is the probability mass function such that \sum_{\omega\in\Omega} p(\omega) = 1
	*/
	Discrete,

};

/*!
	Evaluated PDF value.
	Represents the evaluation of the probability density function (PDF).
	\tparam T Internal value type.
*/
template <typename T>
struct TPDFEval
{

	TPDFEval()
		: v(T(0))
		, measure(ProbabilityMeasure::None)
	{

	}

	TPDFEval(const T& v, ProbabilityMeasure measure)
		: v(v)
		, measure(measure)

	{
	}

	T v;							//!< Value of the PDF evaluation.
	ProbabilityMeasure measure;		//!< Probability measure that the PDF is defined.

};

LM_MATH_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_MATH_PDF_H