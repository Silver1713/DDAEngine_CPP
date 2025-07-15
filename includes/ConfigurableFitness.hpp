#pragma once

#include "ParameterConfig.hpp"
#include "MetricManager.hpp"
#include <functional>
#include <memory>

class ConfigurableFitness {
private:
    FitnessConfig config;
    
    // Evaluation functions for different types
    float evaluateDistance(float value, const FitnessMetricConfig& metricConfig) const;
    float evaluateTarget(float value, const FitnessMetricConfig& metricConfig) const;
    float evaluateMinimize(float value, const FitnessMetricConfig& metricConfig) const;
    float evaluateMaximize(float value, const FitnessMetricConfig& metricConfig) const;
    
    // Normalization helper
    float normalize(float value, float min, float max) const;
    
public:
    ConfigurableFitness() = default;
    explicit ConfigurableFitness(const FitnessConfig& fitnessConfig);
    
    // Configuration management
    void setConfig(const FitnessConfig& fitnessConfig);
    const FitnessConfig& getConfig() const { return config; }
	FitnessConfig& getConfig() { return config; }
    
    // Main fitness evaluation
    float evaluateFitness(const ParameterValues& parameters, const MetricManager& metrics) const;
    
    // Individual metric evaluation
    float evaluateMetric(const std::string& metricName, float value) const;
    
    // Validation
    bool isValidConfig() const;
    
    // Load from file
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;
    
    // Debug information
    std::string getEvaluationBreakdown(const ParameterValues& parameters, const MetricManager& metrics) const;
    
    // Create fitness function for genetic algorithm
    std::function<float(const ParameterValues&, const MetricManager&)> createFitnessFunction() const;
};

// Utility functions for common fitness patterns
namespace FitnessUtils {
    // Create a fitness config for different game types
    FitnessConfig createShooterGameFitness();
    FitnessConfig createPuzzleGameFitness();
    FitnessConfig createPlatformerGameFitness();
    FitnessConfig createRPGGameFitness();
    
    // Validate metric availability
    bool validateMetricsAvailable(const FitnessConfig& config, const MetricManager& metrics);
}