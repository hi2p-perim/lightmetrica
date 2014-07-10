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

#include "common.h"
#include "asset.h"
#include <functional>
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

class ConfigNode;

/*!
	Collection of assets.
	The asset collection class for the asset management.
*/
class LM_PUBLIC_API Assets
{
public:

	Assets() {}
	virtual ~Assets() {}

private:

	LM_DISABLE_COPY_AND_MOVE(Assets);

public:

	/*!
		Get an asset by name. 
		Returns nullptr if not found.
		\param name Name of the asset.
		\return Asset instance. 
	*/
	virtual Asset* GetAssetByName(const std::string& name) const = 0;

	/*!
		Resolve reference to the asset.
		Resolve reference to an asset using \a ref attribute.
		Returns nullptr if \a ref attribute is not found.
		\param node A XML element which consists of the \a ref attribute.
		\param type Target asset type, e.g. triangle_mesh.
		\return Resolved asset.
	*/
	virtual Asset* ResolveReferenceToAsset(const ConfigNode& node, const std::string& type) const;

	/*!
		Resolve reference to the asset.
		Template version of ResolveReferenceToAsset.
		\tparam AssetInterfaceType Asset interface type.
		\param node A XML element which consists of the \a ref attribute.
		\return Resolved asset.
	*/
	template <typename AssetInterfaceType>
	AssetInterfaceType* ResolveReferenceToAsset(const ConfigNode& node) const
	{
		LM_ASSET_CHECK_IS_VALID_INTERFACE(AssetInterfaceType);
		return dynamic_cast<AssetInterfaceType*>(ResolveReferenceToAsset(node, AssetInterfaceType::InterfaceTypeName()));
	}

public:

	/*!
		Connect to ReportProgress signal.
		The signal is emitted when the progress of asset loading is changed.
		\param func Slot function.
	*/
	virtual boost::signals2::connection Connect_ReportProgress(const std::function<void (double, bool)>& func) = 0;

};

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_ASSETS_H