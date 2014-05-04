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
