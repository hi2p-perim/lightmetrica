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
#ifndef LIB_LIGHTMETRICA_TEST_STUB_ASSET_H
#define LIB_LIGHTMETRICA_TEST_STUB_ASSET_H

#include "common.h"
#include <lightmetrica/asset.h>

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

class StubAsset : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset", "stub_assets");
	LM_ASSET_NO_DEPENDENCIES();

};

class StubAsset_Success : public StubAsset
{
public:

	LM_COMPONENT_IMPL_DEF("success");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

class StubAsset_FailOnCreate : public StubAsset
{
public:

	LM_COMPONENT_IMPL_DEF("fail_on_create");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return false; }

};

// --------------------------------------------------------------------------------

class StubAsset_A : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_a", "stub_assets_a");
	LM_ASSET_NO_DEPENDENCIES();

};

class StubAsset_A_Impl : public StubAsset_A
{
public:

	LM_COMPONENT_IMPL_DEF("a");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

class StubAsset_B : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_b", "stub_assets_b");
	LM_ASSET_DEPENDENCIES("stub_asset_a");

};

class StubAsset_B_Impl : public StubAsset_B
{
public:

	LM_COMPONENT_IMPL_DEF("b");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

class StubAsset_C : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_c", "stub_assets_c");
	LM_ASSET_DEPENDENCIES("stub_asset_a", "stub_asset_b");

};

class StubAsset_C_Impl : public StubAsset_C
{
public:

	LM_COMPONENT_IMPL_DEF("c");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

class StubAsset_D : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_d", "stub_assets_d");
	LM_ASSET_DEPENDENCIES("stub_asset_a", "stub_asset_b", "stub_asset_c");

};

class StubAsset_D_Impl : public StubAsset_D
{
public:

	LM_COMPONENT_IMPL_DEF("d");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

// --------------------------------------------------------------------------------

class StubAsset_E : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_e", "stub_assets_e");
	LM_ASSET_DEPENDENCIES("stub_asset_f");

};

class StubAsset_E_Impl : public StubAsset_E
{
public:

	LM_COMPONENT_IMPL_DEF("e");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

class StubAsset_F : public Asset
{
public:

	LM_ASSET_INTERFACE_DEF("stub_asset_f", "stub_assets_f");
	LM_ASSET_DEPENDENCIES("stub_asset_e");

};

class StubAsset_F_Impl : public StubAsset_F
{
public:

	LM_COMPONENT_IMPL_DEF("e");
	virtual bool Load( const ConfigNode& node, const Assets& assets ) { return true; }

};

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_TEST_STUB_ASSET_H
