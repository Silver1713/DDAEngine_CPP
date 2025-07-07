#pragma once
#include "MetricManager.hpp"
#include "nlohmann/json.hpp"
#include <optional>
#include <variant>
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

void MetricManager::addMetric(const std::string& name, DDAMetricType type)
{
	// Create metrics with appropriate data types based on their usage
	if (name == "player_deaths" || name == "enemies_killed" || name == "powerups_collected") {
		CreateMetric(name, type, 0);  // int for count-based metrics
	} else {
		CreateMetric(name, type, 0.0f);  // float for other metrics
	}
}

void MetricManager::clearAll()
{
	metrics.clear();
}

std::optional<std::variant<int, double>> MetricManager::getComputedValue(const std::string& name) const
{
	auto it = metrics.find(name);
	if (it == metrics.end())
	{
		return std::nullopt;
	}
	
	Metric* metric = it->second.get();
	DDAMetricType type = metric->getType();
	
	// Try to cast to different MetricData types
	if (MetricData<int>* intMetric = dynamic_cast<MetricData<int>*>(metric))
	{
		switch (type)
		{
			case DDAMetricType::SUM:
			{
				int sum = 0;
				for (const auto& val : intMetric->dataStore.data)
				{
					sum += val;
				}
				return sum;
			}
			case DDAMetricType::COUNT:
				return static_cast<int>(intMetric->dataStore.data.size());
			case DDAMetricType::AVERAGE:
				if (!intMetric->dataStore.data.empty())
				{
					return static_cast<double>(intMetric->dataStore.average());
				}
				break;
			default:
				return intMetric->value;
		}
	}
	else if (MetricData<float>* floatMetric = dynamic_cast<MetricData<float>*>(metric))
	{
		switch (type)
		{
			case DDAMetricType::SUM:
			{
				double sum = 0.0;
				for (const auto& val : floatMetric->dataStore.data)
				{
					sum += val;
				}
				return sum;
			}
			case DDAMetricType::COUNT:
				return static_cast<int>(floatMetric->dataStore.data.size());
			case DDAMetricType::AVERAGE:
				if (!floatMetric->dataStore.data.empty())
				{
					return static_cast<double>(floatMetric->dataStore.average());
				}
				break;
			default:
				return static_cast<double>(floatMetric->value);
		}
	}
	else if (MetricData<double>* doubleMetric = dynamic_cast<MetricData<double>*>(metric))
	{
		switch (type)
		{
			case DDAMetricType::SUM:
			{
				double sum = 0.0;
				for (const auto& val : doubleMetric->dataStore.data)
				{
					sum += val;
				}
				return sum;
			}
			case DDAMetricType::COUNT:
				return static_cast<int>(doubleMetric->dataStore.data.size());
			case DDAMetricType::AVERAGE:
				if (!doubleMetric->dataStore.data.empty())
				{
					return doubleMetric->dataStore.average();
				}
				break;
			default:
				return doubleMetric->value;
		}
	}
	
	return std::nullopt;
}

size_t MetricManager::getMetricCount(const std::string& name) const
{
	auto it = metrics.find(name);
	if (it == metrics.end())
	{
		return 0;
	}
	
	Metric* metric = it->second.get();
	
	// Try to cast to different MetricData types
	if (MetricData<int>* intMetric = dynamic_cast<MetricData<int>*>(metric))
	{
		return intMetric->dataStore.data.size();
	}
	else if (MetricData<float>* floatMetric = dynamic_cast<MetricData<float>*>(metric))
	{
		return floatMetric->dataStore.data.size();
	}
	else if (MetricData<double>* doubleMetric = dynamic_cast<MetricData<double>*>(metric))
	{
		return doubleMetric->dataStore.data.size();
	}
	
	return 0;
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
