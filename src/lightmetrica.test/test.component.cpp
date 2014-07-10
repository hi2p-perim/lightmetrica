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
#include <lightmetrica/component.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubComponentInterface : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("stub_interface");

public:

	virtual int F() = 0;

};

class StubComponentImpl_1 : public StubComponentInterface
{
public:

	LM_COMPONENT_IMPL_DEF("stub_impl");

public:

	virtual int F() { return 42; }

};

class StubComponentImpl_2 : public StubComponentInterface
{
public:

	virtual int F() { return 43; }

};

class ComponentFactoryTest : public TestBase {};

TEST_F(ComponentFactoryTest, HasMemberFunction)
{
	EXPECT_EQ(true, has_member_function_ImplTypeName<StubComponentImpl_1>::value);
	EXPECT_EQ(false, has_member_function_ImplTypeName<StubComponentImpl_2>::value);
}

TEST_F(ComponentFactoryTest, RegisterAndCreate)
{
	EXPECT_TRUE(ComponentFactory::Register(StubComponentInterface::InterfaceTypeName(), StubComponentImpl_1::ImplTypeName(), [](){ return new StubComponentImpl_1; }));
	EXPECT_TRUE(ComponentFactory::CheckRegistered(StubComponentInterface::InterfaceTypeName(), StubComponentImpl_1::ImplTypeName()));

	auto* inst = ComponentFactory::Create<StubComponentInterface>(StubComponentImpl_1::ImplTypeName());
	EXPECT_EQ(42, inst->F());

	LM_SAFE_DELETE(inst);
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
