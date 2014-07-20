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
#ifndef LIB_LIGHTMETRICA_ASSETS_H
#define LIB_LIGHTMETRICA_ASSETS_H

#include "component.h"
#include "asset.h"
#include <functional>
#include <vector>
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class Asset;
class ConfigNode;

/*!
	Collection of assets.
	The asset collection class for the asset management.
*/
class Assets : public Component
{
public:

	LM_COMPONENT_INTERFACE_DEF("assets");

public:

	Assets() {}
	virtual ~Assets() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Assets);

public:

	/*!
		Load assets from XML element.
		Parse the element #node and register assets.
		\param node A XML element which consists of the \a assets element.
		\retval true Succeeded to load assets.
		\retval false Failed to load assets.
	*/
	virtual bool Load(const ConfigNode& node) = 0;

	/*!
		Get an asset by name. 
		Returns nullptr if not found.
		\param name Name of the asset.
		\return Asset instance. 
	*/
	virtual Asset* GetAssetByName(const std::string& name) const = 0;

	/*!
		Connect to ReportProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) = 0;

	/*!
		Register an interface for assets.
		Register an component interface class for assets creation.
		The class must inherit #Assets and specify dependencies
		to the other asset types with LM_ASSET_DEPENDENCIES macro.
		Fails if the factory with same name is already registered.
		\param interfaceName Name of component interface class which is inherited from Asset.
		\param interfaceGroupName Name of interface group. e.g. \a triangle_meshes for \a triangle_mesh type.
		\param dependencies Dependent asset interface names.
		\retval true Succeeded to register.
		\retval false Failed to register.
		\sa LM_ASSET_DEPENDENCIES
	*/
	virtual bool RegisterInterface(const std::string& interfaceName, const std::string& interfaceGroupName, const std::vector<std::string>& dependencies) = 0;

	/*!
		Register an interface for assets.
		Register an component interface class for assets creation.
		This template version statically checks the requirement for types.
		\tparam AssetInterfaceType Type of component interface class which is inherited from Asset.
		\retval true Succeeded to register.
		\retval false Failed to register.
		\sa LM_ASSET_DEPENDENCIES
	*/
	template <typename AssetInterfaceType>
	bool RegisterInterface();

public:

	/*!
		Resolve reference to the asset.
		Resolve reference to an asset using \a ref attribute.
		Returns nullptr if \a ref attribute is not found.
		\param node A XML element which consists of the \a ref attribute.
		\param type Target asset type, e.g. triangle_mesh.
		\return Resolved asset.
	*/
	LM_PUBLIC_API Asset* ResolveReferenceToAsset(const ConfigNode& node, const std::string& type) const;

	/*!
		Resolve reference to the asset.
		Template version of ResolveReferenceToAsset.
		\tparam AssetInterfaceType Asset interface type.
		\param node A XML element which consists of the \a ref attribute.
		\return Resolved asset.
	*/
	template <typename AssetInterfaceType>
	AssetInterfaceType* ResolveReferenceToAsset(const ConfigNode& node) const;

};

LM_NAMESPACE_END

#include "assets.inl"

#endif // LIB_LIGHTMETRICA_ASSETS_H