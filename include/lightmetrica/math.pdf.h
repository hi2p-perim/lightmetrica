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