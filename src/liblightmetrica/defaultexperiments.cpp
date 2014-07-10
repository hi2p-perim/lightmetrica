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
#include <lightmetrica/defaultexperiments.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/experiment.h>

LM_NAMESPACE_BEGIN

class DefaultExperiments::Impl
{
public:

	Impl();

public:

	bool Configure( const ConfigNode& node, const Assets& assets );
	void Notify( const std::string& type );
	void UpdateParam( const std::string& name, const void* param );
	bool CheckConfigured() { return configured; }

public:

	bool LoadExperiments(const std::vector<Experiment*>& experiments);
	const Experiment* ExperimentByName(const std::string& name) const;

private:

	bool configured;

	// Instances
	std::vector<std::unique_ptr<Experiment>> experiments;
	std::unordered_map<std::string, size_t> experimentIndexMap;

};

DefaultExperiments::Impl::Impl()
	: configured(false)
{

}

bool DefaultExperiments::Impl::Configure( const ConfigNode& node, const Assets& assets )
{
	if (configured)
	{
		LM_LOG_ERROR("Already configured");
		return false;
	}

	// Check for 'experiments' element
	if (node.Name() != "experiments")
	{
		LM_LOG_ERROR("Invalid element name '"+ node.Name() + "' (expected 'experiments')");
		return false;
	}

	// Configure experiments
	experiments.clear();
	experimentIndexMap.clear();

	for (auto experimentNode = node.FirstChild(); !experimentNode.Empty(); experimentNode = experimentNode.NextChild())
	{
		// Element name must be 'experiment'
		if (experimentNode.Name() != "experiment")
		{
			LM_LOG_ERROR("Invalid element name '" + experimentNode.Name() + "' (expected 'experiment')");
			return false;
		}

		// Type of the experiment
		auto typeAttribute = experimentNode.AttributeValue("type");
		if (typeAttribute.empty())
		{
			LM_LOG_ERROR("Missing attribute 'type'");
			return false;
		}

		// Create and configure the experiment
		{
			LM_LOG_INFO("Processing experiment (type : '" + typeAttribute + "')");
			LM_LOG_INDENTER();

			// Check if the experiment with same type is already registered
			if (experimentIndexMap.find(typeAttribute) != experimentIndexMap.end())
			{
				LM_LOG_ERROR("Experiment type '" + typeAttribute + "' is already registered");
				return false;
			}

			// Create an experiment
			std::unique_ptr<Experiment> experiment(ComponentFactory::Create<Experiment>(typeAttribute));
			if (experiment == nullptr)
			{
				LM_LOG_ERROR("Failed to create experiment (type : '" + typeAttribute + "')");
				return false;
			}

			// Configure
			if (!experiment->Configure(experimentNode, assets))
			{
				LM_LOG_ERROR("Failed to configure experiment (type : '" + typeAttribute + "'");
				return false;
			}

			// Register the instance
			experimentIndexMap[typeAttribute] = experiments.size();
			experiments.emplace_back(std::move(experiment));
		}
	}

	configured = true;
	return true;
}

void DefaultExperiments::Impl::Notify( const std::string& type )
{
	// TODO : Implement destination selected notification
	for (auto& experiment : experiments)
	{
		experiment->Notify(type);
	}
}

void DefaultExperiments::Impl::UpdateParam( const std::string& name, const void* param )
{
	for (auto& experiment : experiments)
	{
		experiment->UpdateParam(name, param);
	}
}

bool DefaultExperiments::Impl::LoadExperiments( const std::vector<Experiment*>& experiments )
{
	if (configured)
	{
		LM_LOG_ERROR("Already configured");
		return false;
	}

	this->experiments.clear();
	experimentIndexMap.clear();

	for (size_t i = 0; i < experiments.size(); i++)
	{
		auto& experiment = experiments[i];
		
		if (experimentIndexMap.find(experiment->ComponentImplTypeName()) != experimentIndexMap.end())
		{
			LM_LOG_ERROR("Experiment type '" + experiment->ComponentImplTypeName() + "' is already registered");
			return false;
		}

		experimentIndexMap[experiment->ComponentImplTypeName()] = i;
		this->experiments.emplace_back(experiment);
	}

	configured = true;
	return true;
}

const Experiment* DefaultExperiments::Impl::ExperimentByName( const std::string& name ) const
{
	if (experimentIndexMap.find(name) == experimentIndexMap.end())
	{
		LM_LOG_ERROR("Experiment '" + name + "' is not found");
		return nullptr;
	}

	return experiments[experimentIndexMap.at(name)].get();
}

// --------------------------------------------------------------------------------

DefaultExperiments::DefaultExperiments()
	: p(new Impl)
{

}

DefaultExperiments::~DefaultExperiments()
{
	LM_SAFE_DELETE(p);
}

bool DefaultExperiments::Configure( const ConfigNode& node, const Assets& assets )
{
	return p->Configure(node, assets);
}

void DefaultExperiments::Notify( const std::string& type )
{
	p->Notify(type);
}

void DefaultExperiments::UpdateParam( const std::string& name, const void* param )
{
	p->UpdateParam(name, param);
}

bool DefaultExperiments::CheckConfigured()
{
	return p->CheckConfigured();
}

bool DefaultExperiments::LoadExperiments( const std::vector<Experiment*>& experiments )
{
	return p->LoadExperiments(experiments);
}

const Experiment* DefaultExperiments::ExperimentByName( const std::string& name ) const
{
	return p->ExperimentByName(name);
}

LM_NAMESPACE_END