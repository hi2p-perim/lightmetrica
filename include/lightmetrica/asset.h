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
		\param true Succeeded to load.
		\param false Failed to load.
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