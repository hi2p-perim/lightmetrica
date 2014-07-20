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

#include "assets.h"

LM_NAMESPACE_BEGIN

template <typename AssetInterfaceType>
bool Assets::RegisterInterface()
{
	LM_ASSET_CHECK_IS_VALID_INTERFACE(AssetInterfaceType);

	// Create dependency list
	size_t n;
	const char** deps = AssetInterfaceType::GetAssetDependencies(n);
	std::vector<std::string> dependencies;
	for (size_t i = 0; i < n; i++)
	{
		dependencies.push_back(deps[i]);
	}
		
	// Register
	std::string interfaceTypeName = AssetInterfaceType::InterfaceTypeName();
	std::string groupName = AssetInterfaceType::InterfaceGroupName();
	return RegisterInterface(interfaceTypeName, groupName, dependencies);
}

template <typename AssetInterfaceType>
AssetInterfaceType* Assets::ResolveReferenceToAsset(const ConfigNode& node) const
{
	LM_ASSET_CHECK_IS_VALID_INTERFACE(AssetInterfaceType);
	return dynamic_cast<AssetInterfaceType*>(ResolveReferenceToAsset(node, AssetInterfaceType::InterfaceTypeName()));
}

LM_NAMESPACE_END