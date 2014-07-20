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
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/pugihelper.h>
#include <thread>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

LM_NAMESPACE_BEGIN

struct AssetInterfaceInfo
{

	std::string group;
	std::vector<std::string> dependencies;

	AssetInterfaceInfo() {}
	AssetInterfaceInfo(const std::string& group, const std::vector<std::string>& dependencies)
		: group(group)
		, dependencies(dependencies)
	{}

};

/*!
	An entry for the asset factory.
	This structure is used for registering asset factory to #Assets class.
	\sa Assets::RegisterAssetFactory
*/
class AssetsImpl : public Assets
{
public:

	LM_COMPONENT_IMPL_DEF("default");

public:

	~AssetsImpl();

public:

	virtual bool Load( const ConfigNode& node );
	virtual Asset* GetAssetByName( const std::string& name ) const;
	virtual boost::signals2::connection Connect_ReportProgress( const std::function<void (double, bool ) >& func) { return signal_ReportProgress.connect(func); }
	virtual bool RegisterInterface( const std::string& interfaceName, const std::string& interfaceGroupName, const std::vector<std::string>& dependencies );

private:

	boost::signals2::signal<void (double, bool)> signal_ReportProgress;

	std::unordered_map<std::string, AssetInterfaceInfo> interfaceInfo;		// Registered interfaces
	std::vector<Asset*> assetInstances;										// Asset instances
	std::vector<ConfigNode> assetInstanceNodes;								// Config nodes for corresponding assets
	boost::unordered_map<std::string, size_t> assetIndexMap;				// For search query

};

AssetsImpl::~AssetsImpl()
{
	for (auto& v : assetInstances)
		LM_SAFE_DELETE(v);
}

bool AssetsImpl::Load( const ConfigNode& node )
{
	// Element name must be 'assets'
	if (node.Name() != "assets")
	{
		LM_LOG_ERROR("Invalid element name '" + node.Name() + "' (expected 'assets')");
		return false;
	}

	std::vector<std::string> interfaces;
	std::deque<size_t> orderedIndices;
	{
		LM_LOG_INFO("Stage : Resolving dependency");

		// Copy the contents of interface set to a vector
		for (auto& kv : interfaceInfo)
		{
			interfaces.push_back(kv.first);
		}
		boost::unordered_map<std::string, size_t> interfaceNameIndexMap;
		for (size_t i = 0; i < interfaces.size(); i++)
		{
			interfaceNameIndexMap[interfaces[i]] = i;
		}

		// Create dependency graph
		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
		Graph g(interfaces.size());
		for (size_t i = 0; i < interfaces.size(); i++)
		{
			auto& info = interfaceInfo[interfaces[i]];
			for (auto& dependency : info.dependencies)
			{
				auto it = interfaceNameIndexMap.find(dependency);
				if (it == interfaceNameIndexMap.end())
				{
					LM_LOG_ERROR("Invalid dependency : '" + interfaces[i] + "' -> '" + dependency + "'");
					return false;
				}
				boost::add_edge(it->second, i, g);
			}
		}

		try
		{
			// Perform topological sort
			boost::topological_sort(g, std::front_inserter(orderedIndices));
		}
		catch (const boost::not_a_dag&)
		{
			LM_LOG_ERROR("Detected inappropriate dependency. Dependency graph is not a DAG.");
			return false;
		}

#ifdef LM_DEBUG_MODE
		{
			LM_LOG_DEBUG("Resolved dependency");
			LM_LOG_INDENTER();
			for (size_t i : orderedIndices)
			{
				LM_LOG_DEBUG(interfaces[i]);
			}	
		}
#endif
	}

	{
		LM_LOG_INFO("Stage : Finding assets");
		LM_LOG_INDENTER();

		// By priority, find the child element under 'assets', and
		// find corresponding asset factory and create asset instances.
		for (auto& interfaceIndex : orderedIndices)
		{
			// Find the element under 'assets'
			auto interfaceName  = interfaces[interfaceIndex];
			auto assetGroupName = interfaceInfo[interfaceName].group;
			auto assetGroupNode = node.Child(assetGroupName);
			if (assetGroupNode.Empty())
			{
				LM_LOG_WARN("Missing asset group '" + assetGroupName + "'. Skipping");
				continue;
			}

			LM_LOG_INFO(boost::str(boost::format("Processing asset group '%s'") % assetGroupName));
			LM_LOG_INDENTER();

			// For each child of the node, create an instance of the asset
			for (auto assetNode = assetGroupNode.FirstChild(); !assetNode.Empty(); assetNode = assetNode.NextChild())
			{
				// Check asset name
				auto name = assetNode.Name();
				if (name != interfaceName)
				{
					LM_LOG_ERROR(boost::str(boost::format("Invlaid element name '%s' (expected '&s')") % name % interfaceName));
					return false;
				}

				// Type of the asset
				auto typeAttribute = assetNode.AttributeValue("type");
				if (typeAttribute.empty())
				{
					LM_LOG_ERROR("Missing attribute 'type'.");
					return false;
				}

				auto idAttribute = assetNode.AttributeValue("id");
				if (idAttribute.empty())
				{
					LM_LOG_ERROR("Missing attribute 'id'.");
					return false;
				}

				{
					LM_LOG_INFO("Processing asset (id : '" + idAttribute + "', type : '" + typeAttribute + "')");
					LM_LOG_INDENTER();

					// Check if the 'id' is already registered
					if (assetIndexMap.find(idAttribute) != assetIndexMap.end())
					{
						LM_LOG_ERROR("ID '" + idAttribute + "' is already registered.");
						return false;
					}

					auto* asset = dynamic_cast<Asset*>(ComponentFactory::Create(interfaceName, typeAttribute));
					if (asset == nullptr)
					{
						LM_LOG_ERROR("Failed to create the asset");
						return false;
					}

					// Set ID
					asset->SetID(idAttribute);

					// Register the instance
					assetIndexMap[idAttribute] = assetInstances.size();
					assetInstances.push_back(asset);
					assetInstanceNodes.push_back(assetNode);
				}
			}
		}

		LM_LOG_INFO("Successfully found " + std::to_string(assetInstances.size()) + " assets");
	}

	{
		LM_LOG_INFO("Stage : Loading assets");
		LM_LOG_INDENTER();

		signal_ReportProgress(0, false);

		for (size_t i = 0; i < assetInstances.size(); i++)
		{
			auto* asset = assetInstances[i];

			LM_LOG_INFO(boost::str(boost::format("Loading asset (id : '%s', type : '%s')") % asset->ID() % asset->ComponentInterfaceTypeName()));
			LM_LOG_INDENTER();

			// Load
			if (!asset->Load(assetInstanceNodes[i], *this))
			{
				LM_LOG_ERROR("Failed to load the asset.");
				return false;
			}

			// Update progress
			signal_ReportProgress(static_cast<double>(i+1) / assetInstances.size(), i+1 == assetInstances.size());
		}

		LM_LOG_INFO("Successfully loaded " + std::to_string(assetInstances.size()) + " assets");
	}

	return true;
}

Asset* AssetsImpl::GetAssetByName( const std::string& name ) const
{
	return assetIndexMap.find(name) == assetIndexMap.end() ? nullptr : assetInstances[assetIndexMap.at(name)];
}

bool AssetsImpl::RegisterInterface( const std::string& interfaceName, const std::string& interfaceGroupName, const std::vector<std::string>& dependencies )
{
	if (interfaceInfo.find(interfaceName) != interfaceInfo.end())
	{
		LM_LOG_ERROR("Component interface '" + interfaceName + "' is already registered");
		return false;
	}

	interfaceInfo[interfaceName] = AssetInterfaceInfo(interfaceGroupName, dependencies);
	return true;
}

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

LM_COMPONENT_REGISTER_IMPL(AssetsImpl, Assets);

LM_NAMESPACE_END