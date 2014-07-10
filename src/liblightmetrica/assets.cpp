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
#include <lightmetrica/assets.h>
#include <lightmetrica/asset.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/confignode.h>

LM_NAMESPACE_BEGIN

Asset* Assets::ResolveReferenceToAsset( const ConfigNode& node, const std::string& type ) const
{
	// The element must have 'ref' attribute
	auto refAttr = node.AttributeValue("ref");
	if (refAttr.empty())
	{
		LM_LOG_ERROR("'" + node.Name() + "' element must have 'ref' attribute");
		return nullptr;
	}

	// Find the light specified by 'ref'
	auto* asset = GetAssetByName(refAttr);
	if (!asset)
	{
		LM_LOG_ERROR("The asset referenced by '" + refAttr + "' is not found");
		return nullptr;
	}

	// Check type
	if (asset->ComponentInterfaceTypeName() != type)
	{
		LM_LOG_ERROR("Invalid reference to asset type '" + asset->ComponentInterfaceTypeName() + "' (expected '" + type + "')");
		return nullptr;
	}

	return asset;
}

LM_NAMESPACE_END