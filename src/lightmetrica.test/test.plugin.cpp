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
#include <lightmetrica.test/base.math.h>
#include <lightmetrica/component.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class PluginTest : public TestBase
{
public:

	virtual void SetUp()
	{
		TestBase::SetUp();
		ComponentFactory::UnloadPlugins();
		ComponentFactory::LoadPlugins(".");
	}

};

TEST_F(PluginTest, Load)
{
	auto* instance = ComponentFactory::Create("bsdf", "plugin.testbsdf");
	ASSERT_NE(instance, nullptr);
	EXPECT_EQ("bsdf", instance->ComponentInterfaceTypeName());
	EXPECT_EQ("plugin.testbsdf", instance->ComponentImplTypeName());
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
