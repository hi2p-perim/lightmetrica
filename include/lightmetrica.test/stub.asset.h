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
#ifndef LIB_LIGHTMETRICA_TEST_STUB_ASSET_H
#define LIB_LIGHTMETRICA_TEST_STUB_ASSET_H

#include "common.h"
#include <lightmetrica/asset.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/confignode.h>

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
	virtual bool Load( const ConfigNode& node, const Assets& assets )
	{
		return true;
	}

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
	virtual bool Load( const ConfigNode& node, const Assets& assets )
	{
		ConfigNode child;
		Asset* p;

		// Check if 'stub_asset_a' (whose ID is expected to be set to 'a') is properly loaded
		child = node.Child("stub_asset_a");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_a");
		EXPECT_NE(nullptr, p);

		return p != nullptr;
	}

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
	virtual bool Load( const ConfigNode& node, const Assets& assets )
	{
		ConfigNode child;
		Asset* p;
		
		child = node.Child("stub_asset_a");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_a");
		EXPECT_NE(nullptr, p);

		child = node.Child("stub_asset_b");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_b");
		EXPECT_NE(nullptr, p);

		return p != nullptr;
	}

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
	virtual bool Load( const ConfigNode& node, const Assets& assets )
	{
		ConfigNode child;
		Asset* p;

		child = node.Child("stub_asset_a");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_a");
		EXPECT_NE(nullptr, p);

		child = node.Child("stub_asset_b");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_b");
		EXPECT_NE(nullptr, p);

		child = node.Child("stub_asset_c");
		EXPECT_FALSE(child.Empty());
		p = assets.ResolveReferenceToAsset(child, "stub_asset_c");
		EXPECT_NE(nullptr, p);

		return p != nullptr;
	}

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
