#ifndef METRIC_MANAGER_HPP
#define METRIC_MANAGER_HPP
#include <memory>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <optional>
#include <variant>

#include "Metric.hpp"
#include "MetricData.hpp"

class MetricManager
{
	static MetricManager* instance; // Singleton instance

public:

	std::unordered_map<std::string, std::unique_ptr<Metric>> metrics; // Set of all metrics

	MetricManager() = default; // Private constructor for singleton
	MetricManager(const MetricManager&) = delete; // Delete copy constructor
	MetricManager& operator=(const MetricManager&) = delete; // Delete assignment operator

	static MetricManager* getInstance();

	Metric* getMetric(std::string const& name);
	Metric* getMetric(const std::string& name) const;

	template <typename T >
	void CreateMetric(const std::string& name, DDAMetricType type, T value, bool isBayesian = false);
	template <typename T>
	MetricData<T>& getMetricData(const std::string& name) const;

	void addMetric(const std::string& name, DDAMetricType type);
	
	template <typename T>
	void pushMetric(const std::string& name, T value);
	
	std::optional<std::variant<int, double>> getComputedValue(const std::string& name) const;
	
	size_t getMetricCount(const std::string& name) const;
	
	void clearAll();

	void RemoveMetric(const std::string& name);



	std::string export_as_json();




};


template <typename T>
void MetricManager::CreateMetric(const std::string& name, DDAMetricType type, T value, bool isBayesian)
{
	std::unique_ptr<MetricData<T>> metricData = std::make_unique<MetricData<T>>(name, type, value, isBayesian);
	metrics.insert({name, std::move(metricData)});

}

template <typename T>
MetricData<T>& MetricManager::getMetricData(const std::string& name) const
{
	MetricData<T>* metricData = dynamic_cast<MetricData<T>*>(getMetric(name));
	if (metricData)
	{
		return *metricData;
	}
	else
	{
		throw std::runtime_error("Metric not found or type mismatch: " + name);
	}
}

template <typename T>
void MetricManager::pushMetric(const std::string& name, T value)
{
	auto it = metrics.find(name);
	if (it != metrics.end())
	{
		MetricData<T>* metricData = dynamic_cast<MetricData<T>*>(it->second.get());
		if (metricData)
		{
			metricData->dataStore.addData(value);
			metricData->value = value;
		}
		else
		{
			throw std::runtime_error("Metric type mismatch for: " + name);
		}
	}
	else
	{
		throw std::runtime_error("Metric not found: " + name);
	}
}


#endif