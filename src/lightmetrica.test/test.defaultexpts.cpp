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

#include "pch.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica.test/stub.config.h>
#include <lightmetrica.test/stub.assets.h>
#include <lightmetrica/defaultexperiments.h>
#include <lightmetrica/experiment.h>

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

	LM_COMPONENT_IMPL_DEF("stub");

public:

	StubExperiment()
		: v(0)
		, notified(false)
	{

	}

public:

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

LM_COMPONENT_REGISTER_IMPL(StubExperiment, Experiment);

// --------------------------------------------------------------------------------

class DefaultExperimentsTest : public TestBase
{
protected:

	StubAssets assets;
	StubConfig config;
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
	EXPECT_TRUE(expts.Configure(config.LoadFromStringAndGetFirstChild(ExperimentNode_1), assets));

	LM_EXPT_NOTIFY(expts, "test");
	
	const auto* expt = dynamic_cast<const StubExperiment*>(expts.ExperimentByName("stub"));
	EXPECT_TRUE(expt->notified);
}

TEST_F(DefaultExperimentsTest, UpdateParam)
{
	EXPECT_TRUE(expts.Configure(config.LoadFromStringAndGetFirstChild(ExperimentNode_1), assets));

	const int v = 42;
	LM_EXPT_UPDATE_PARAM(expts, "test", &v);

	const auto* expt = dynamic_cast<const StubExperiment*>(expts.ExperimentByName("stub"));
	EXPECT_EQ(42, expt->v);
}

TEST_F(DefaultExperimentsTest, LoadExperiments)
{
	std::vector<Experiment*> experiments;
	experiments.push_back(new StubExperiment);

	EXPECT_TRUE(expts.LoadExperiments(experiments));
	EXPECT_TRUE(expts.CheckConfigured());

	EXPECT_EQ(experiments[0], expts.ExperimentByName("stub"));
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
