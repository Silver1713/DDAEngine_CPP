#include "Metric.hpp"

Metric::Metric(std::string name, DDAMetricType type, bool isBayesian)
	: identifier(std::move(name)), type(type), BayesianInfer(isBayesian)
{}

std::string Metric::getName() const
{
	return identifier;
}


DDAMetricType Metric::getType() const
{
	return type;
}


bool Metric::isBayesianInfer() const
{
	return BayesianInfer;
}



