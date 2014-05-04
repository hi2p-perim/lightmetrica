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
#ifndef LIB_LIGHTMETRICA_ASSET_H
#define LIB_LIGHTMETRICA_ASSET_H

#include "component.h"

LM_NAMESPACE_BEGIN

class Assets;
class ConfigNode;

/*!
	Asset.
	A base class for assets.
*/
class Asset : public Component
{
public:

	Asset() {}
	virtual ~Asset() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Asset);

public:

	/*!
		Load an asset.
		Configure and initialize the asset by the XML elements given by #node.
		Some assets have references to the other assets, so #assets is also required.
		Dependent asset must be loaded beforehand.
		\param node XML node for the configuration.
		\param assets Asset manager.
	*/
	virtual bool Load(const ConfigNode& node, const Assets& assets) = 0;

	/*!
		Get ID of the asset.
		\return ID of the asset.
	*/
	LM_PUBLIC_API std::string ID() const;

public:

	/*!
		Set ID of the asset.
		This is an internal function.
		\param id ID of the asset.
	*/
	LM_HIDDEN_API void SetID(const std::string& id);

private:

	std::string id;

};

LM_NAMESPACE_END

#define LM_ASSET_INTERFACE_DEF(Name, GroupName)		\
	LM_COMPONENT_INTERFACE_DEF(Name);				\
	static const char* InterfaceGroupName() { return GroupName; }

#define LM_ASSET_DEPENDENCIES(...)								\
	static const char** GetAssetDependencies(size_t& n)			\
	{															\
		static const char* deps[] = { __VA_ARGS__ };			\
		n = sizeof(deps) / sizeof(deps[0]);						\
		return deps;											\
	}

#define LM_ASSET_NO_DEPENDENCIES()								\
	static const char** GetAssetDependencies(size_t& n)			\
	{															\
		n = 0;													\
		return nullptr;											\
	}

LM_NAMESPACE_BEGIN
LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(GetAssetDependencies, const char** (*)(size_t&));
LM_COMPONENT_CREATE_HAS_MEMBER_FUNCTION(InterfaceGroupName, const char* (*)());
LM_NAMESPACE_END

#define LM_ASSET_CHECK_IS_VALID_INTERFACE(AssetInterfaceType)							\
	LM_COMPONENT_CHECK_IS_DERIVED_CLASS(AssetInterfaceType, Asset);						\
	LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(AssetInterfaceType, GetAssetDependencies);	\
	LM_COMPONENT_CHECK_HAS_MEMBER_FUNCTION(AssetInterfaceType, InterfaceGroupName);

#endif // LIB_LIGHTMETRICA_ASSET_H