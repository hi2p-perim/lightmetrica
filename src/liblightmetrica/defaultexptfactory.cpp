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
#include <lightmetrica/defaultexptfactory.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/expt.recordimage.h>
#include <lightmetrica/expt.recordrmse.h>
#include <lightmetrica/expt.pssmlttraceplot.h>
#include <lightmetrica/expt.pssmltrunningmean.h>
#include <lightmetrica/expt.pssmltlength.h>
#include <lightmetrica/expt.pssmltacceptanceratio.h>

LM_NAMESPACE_BEGIN

Experiment* DefaultExperimentFactory::Create( const std::string& type ) const
{
	if (type == "recordimage")
	{
		return new RecordImageExperiment;
	}
	else if (type == "recordrmse")
	{
		return new RecordRMSEExperiment;
	}
	else if (type == "pssmlttraceplot")
	{
		return new PSSMLTTraceplotExperiment;
	}
	else if (type == "pssmltrunningmean")
	{
		return new PSSMLTRunningMeanExperiment;
	}
	else if (type == "pssmltlength")
	{
		return new PSSMLTLengthExperiment;
	}
	else if (type == "pssmltacceptanceratio")
	{
		return new PSSMLTAcceptanceRatioExperiment;
	}
	else
	{
		LM_LOG_ERROR("Invalid experiment type '" + type + "'");
		return nullptr;
	}
}

LM_NAMESPACE_END