#ifndef METRIC_HPP
#define METRIC_HPP
#include <string>
#include <variant>
//Common Metric class

enum struct DDAMetricType;

class Metric
{
public:
	using MetricDataTypes = std::variant<int, float, double, std::string, char, bool>; // Possible data types for metrics

	std::string identifier; // Unique identifier for the metric
	DDAMetricType type; // Type of metric (e.g., SUM, AVERAGE, etc.)
	bool BayesianInfer = false; // True if the result is a Bayesian inference result


	Metric() = default; // Default constructor for empty initialization
	Metric(std::string name, DDAMetricType type, bool isBayesian);


	virtual std::string getName() const;
	virtual std::string ToString() const = 0; // Pure virtual function to convert metric to string representation

	virtual MetricDataTypes getValue() const = 0; // Pure virtual function to get the value of the metric
	
	
	virtual DDAMetricType getType() const;
	virtual bool isBayesianInfer() const;


	

	virtual ~Metric() = default; // Virtual destructor for proper cleanup
};

#endif