#pragma once
#include "MetricManager.hpp"
#include "nlohmann/json.hpp"
MetricManager* MetricManager::instance = nullptr;


MetricManager* MetricManager::getInstance()
{
	if (!instance)
	{
		instance = new MetricManager();
	}
	return instance;
}

Metric* MetricManager::getMetric(std::string const& name) const
{
	
	bool found = metrics.contains(name);
	if (found)
	{
		return metrics.at(name).get();
	}
	else
	{
		throw std::runtime_error("Metric not found: " + name);
	}
}


Metric* MetricManager::getMetric(std::string const& name)
{
	bool found = metrics.contains(name);
	if (found)
	{
		return metrics.at(name).get();
	}
	else
	{
		throw std::runtime_error("Metric not found: " + name);
	}

}






void MetricManager::RemoveMetric(const std::string& name)
{

	if (metrics.contains(name))
	{
		metrics.erase(name);
	}
	else
	{
		throw std::runtime_error("Metric not found: " + name);
	}

}

std::string MetricManager::export_as_json()
{
	nlohmann::json j;
	for (const auto& [name, metric] : metrics)
	{
		j[name] = metric->ToString(); // Assuming ToString() returns a JSON-compatible string representation
	}
		return j.dump(4); // Pretty print with 4 spaces
	
}
