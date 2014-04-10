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
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/defaultexpts.h>
#include <lightmetrica/exptfactory.h>
#include <lightmetrica/expt.h>

namespace
{

	const std::string ExperimentNode_1 = LM_TEST_MULTILINE_LITERAL(
		<experiments>
			<experiment type="stub" />
		</experiments>
	);

}

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubExperiment : public Experiment
{
public:

	StubExperiment()
		: v(0)
		, notified(false)
	{

	}

public:

	virtual std::string Type() const { return "stub"; }
	virtual bool Configure( const ConfigNode& node, const Assets& assets )  { return true; }
	
	virtual void Notify( const std::string& type )
	{
		if (type == "test")
		{
			notified = true;
		}
	}

	virtual void UpdateParam( const std::string& name, const void* param ) 
	{
		if (name == "test")
		{
			v = *(int*)param;
		}
	}

public:

	int v;
	bool notified;

};

class StubExperimentFactory : public ExperimentFactory
{
public:

	virtual Experiment* Create( const std::string& type ) const
	{
		if (type == "stub") return new StubExperiment;
		else return nullptr;
	}

};

// --------------------------------------------------------------------------------

class DefaultExperimentsTest : public TestBase
{
public:

	DefaultExperimentsTest()
	{
		expts.RegisterExperimentFactory(&factory);
	}

protected:

	StubAssets assets;
	StubConfig config;
	StubExperimentFactory factory;
	DefaultExperiments expts;

};

// --------------------------------------------------------------------------------

TEST_F(DefaultExperimentsTest, Configure)
{
	EXPECT_TRUE(expts.Configure(config.LoadFromStringAndGetFirstChild(ExperimentNode_1), assets));
	EXPECT_TRUE(expts.CheckConfigured());
}

TEST_F(DefaultExperimentsTest, Notify)
{
	// TODO
	EXPECT_TRUE(expts.Configure(config.LoadFromStringAndGetFirstChild(ExperimentNode_1), assets));
	LM_EXPT_NOTIFY(expts, "test");
	EXPECT_TRUE()
}

TEST_F(DefaultExperimentsTest, UpdateParam)
{

}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
