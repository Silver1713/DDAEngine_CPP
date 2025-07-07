#ifndef METRIC_MANAGER_HPP
#define METRIC_MANAGER_HPP
#include <memory>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <unordered_map>

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


#endif