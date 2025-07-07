#include <iostream>

#include "MetricData.hpp"
#include "MetricManager.hpp"

int main()
{

	MetricManager* manager = MetricManager::getInstance();
	manager->CreateMetric("example1", DDAMetricType::AVERAGE, 100, false);
	manager->CreateMetric("example2", DDAMetricType::SUM, "Hello World", true);
	Metric* metric1 = manager->getMetric("example1");
	Metric* metric2 = manager->getMetric("example2");

	// JSON
	std::cout << manager->export_as_json() << std::endl;
	
}
