#include "ConfigurableFitness.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

ConfigurableFitness::ConfigurableFitness(const FitnessConfig& fitnessConfig) : config(fitnessConfig) {}

void ConfigurableFitness::setConfig(const FitnessConfig& fitnessConfig) {
    config = fitnessConfig;
}

float ConfigurableFitness::normalize(float value, float min, float max) const {
    if (max == min) return 0.0f;
    return std::max(0.0f, std::min(1.0f, (value - min) / (max - min)));
}

float ConfigurableFitness::evaluateDistance(float value, const FitnessMetricConfig& metricConfig) const {
    float normalizedValue = normalize(value, metricConfig.minValue, metricConfig.maxValue);
    float normalizedIdeal = normalize(metricConfig.idealValue, metricConfig.minValue, metricConfig.maxValue);
    
    // Distance from ideal (closer is better)
    float distance = std::abs(normalizedValue - normalizedIdeal);
    return 1.0f - distance; // Invert so higher is better
}

float ConfigurableFitness::evaluateTarget(float value, const FitnessMetricConfig& metricConfig) const {
    // Similar to distance but with a steeper penalty curve
    float normalizedValue = normalize(value, metricConfig.minValue, metricConfig.maxValue);
    float normalizedIdeal = normalize(metricConfig.idealValue, metricConfig.minValue, metricConfig.maxValue);
    
    float distance = std::abs(normalizedValue - normalizedIdeal);
    // Use exponential decay for steeper penalty
    return std::exp(-distance * 3.0f);
}

float ConfigurableFitness::evaluateMinimize(float value, const FitnessMetricConfig& metricConfig) const {
    float normalized = normalize(value, metricConfig.minValue, metricConfig.maxValue);
    return 1.0f - normalized; // Lower values are better
}

float ConfigurableFitness::evaluateMaximize(float value, const FitnessMetricConfig& metricConfig) const {
    return normalize(value, metricConfig.minValue, metricConfig.maxValue); // Higher values are better
}

float ConfigurableFitness::evaluateMetric(const std::string& metricName, float value) const {
    // Find the metric configuration
    auto it = std::find_if(config.metrics.begin(), config.metrics.end(),
        [&metricName](const FitnessMetricConfig& metric) {
            return metric.metricName == metricName;
        });
    
    if (it == config.metrics.end()) {
        return 0.0f; // Metric not configured
    }
    
    const FitnessMetricConfig& metricConfig = *it;
    
    if (metricConfig.evaluationType == "distance") {
        return evaluateDistance(value, metricConfig);
    } else if (metricConfig.evaluationType == "target") {
        return evaluateTarget(value, metricConfig);
    } else if (metricConfig.evaluationType == "minimize") {
        return evaluateMinimize(value, metricConfig);
    } else if (metricConfig.evaluationType == "maximize") {
        return evaluateMaximize(value, metricConfig);
    }
    
    return 0.0f;
}

float ConfigurableFitness::evaluateFitness(const ParameterValues& parameters, const MetricManager& metrics) const {
    float fitness = config.baseFitness;
    float totalWeight = 0.0f;
    float weightedScore = 0.0f;
    
    for (const auto& metricConfig : config.metrics) {
        auto metricValue = metrics.getComputedValue(metricConfig.metricName);
        if (!metricValue) {
            continue; // Skip unavailable metrics
        }
        
        float value = 0.0f;
        if (std::holds_alternative<int>(*metricValue)) {
            value = static_cast<float>(std::get<int>(*metricValue));
        } else if (std::holds_alternative<double>(*metricValue)) {
            value = static_cast<float>(std::get<double>(*metricValue));
        }
        
        // Handle special cases for derived metrics
        if (metricConfig.metricName == "player_deaths") {
            // Convert to death rate
            float levelCount = std::max(1.0f, static_cast<float>(metrics.getMetricCount("completion_time")));
            value = value / levelCount;
        }
        
        float metricScore = evaluateMetric(metricConfig.metricName, value);
        
        if (config.aggregationMethod == "weighted_sum") {
            weightedScore += metricScore * metricConfig.weight;
            totalWeight += metricConfig.weight;
        } else if (config.aggregationMethod == "weighted_product") {
            // For product, we need to handle weights differently
            weightedScore += std::log(std::max(0.001f, metricScore)) * metricConfig.weight;
            totalWeight += metricConfig.weight;
        }
    }
    
    if (totalWeight > 0.0f) {
        if (config.aggregationMethod == "weighted_sum") {
            fitness += (weightedScore / totalWeight) * 50.0f; // Scale the contribution
        } else if (config.aggregationMethod == "weighted_product") {
            fitness += std::exp(weightedScore / totalWeight) * 50.0f;
        }
    }
    
    // Add parameter-based penalties/bonuses
    float parameterPenalty = 0.0f;
    
    // Penalize extreme parameter values
    float aggressiveness = parameters.getValue<float>("AI.aggressiveness", 0.5f);
    float enemyDensity = parameters.getValue<float>("PCG.enemyDensity", 0.5f);
    
    if (aggressiveness > 0.9f || aggressiveness < 0.1f) {
        parameterPenalty += 5.0f;
    }
    if (enemyDensity > 0.9f || enemyDensity < 0.1f) {
        parameterPenalty += 5.0f;
    }
    
    fitness -= parameterPenalty;
    
    return std::max(0.0f, fitness);
}

bool ConfigurableFitness::isValidConfig() const {
    if (config.metrics.empty()) {
        return false;
    }
    
    for (const auto& metric : config.metrics) {
        if (metric.metricName.empty()) {
            return false;
        }
        if (metric.weight < 0.0f) {
            return false;
        }
        if (metric.maxValue <= metric.minValue) {
            return false;
        }
    }
    
    return true;
}

bool ConfigurableFitness::loadFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open fitness config file: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        if (j.contains("fitnessConfig")) {
            config.fromJson(j["fitnessConfig"]);
        } else {
            config.fromJson(j);
        }
        
        return isValidConfig();
    } catch (const std::exception& e) {
        std::cerr << "Error loading fitness config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigurableFitness::saveToFile(const std::string& filePath) const {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to create fitness config file: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json j;
        j["fitnessConfig"] = config.toJson();
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving fitness config: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigurableFitness::getEvaluationBreakdown(const ParameterValues& parameters, const MetricManager& metrics) const {
    std::stringstream ss;
    ss << "Fitness Evaluation Breakdown:\n";
    ss << "Base Fitness: " << config.baseFitness << "\n";
    
    float totalContribution = 0.0f;
    
    for (const auto& metricConfig : config.metrics) {
        auto metricValue = metrics.getComputedValue(metricConfig.metricName);
        if (!metricValue) {
            ss << metricConfig.metricName << ": UNAVAILABLE\n";
            continue;
        }
        
        float value = 0.0f;
        if (std::holds_alternative<int>(*metricValue)) {
            value = static_cast<float>(std::get<int>(*metricValue));
        } else if (std::holds_alternative<double>(*metricValue)) {
            value = static_cast<float>(std::get<double>(*metricValue));
        }
        
        float metricScore = evaluateMetric(metricConfig.metricName, value);
        float contribution = metricScore * metricConfig.weight;
        totalContribution += contribution;
        
        ss << metricConfig.metricName << ": " << value 
           << " -> Score: " << metricScore 
           << " (Weight: " << metricConfig.weight 
           << ", Contribution: " << contribution << ")\n";
    }
    
    float finalFitness = evaluateFitness(parameters, metrics);
    ss << "Total Contribution: " << totalContribution << "\n";
    ss << "Final Fitness: " << finalFitness << "\n";
    
    return ss.str();
}

std::function<float(const ParameterValues&, const MetricManager&)> ConfigurableFitness::createFitnessFunction() const {
    return [this](const ParameterValues& params, const MetricManager& metrics) -> float {
        return this->evaluateFitness(params, metrics);
    };
}

// Utility functions implementation
namespace FitnessUtils {

FitnessConfig createShooterGameFitness() {
    FitnessConfig config;
    config.baseFitness = 100.0f;
    config.aggregationMethod = "weighted_sum";
    config.metrics = {
        {"player_deaths", 35.0f, 0.15f, 0.0f, 3.0f, "distance"},
        {"completion_time", 25.0f, 240.0f, 60.0f, 480.0f, "distance"},
        {"accuracy", 20.0f, 0.75f, 0.0f, 1.0f, "distance"},
        {"enemies_killed", 15.0f, 25.0f, 0.0f, 100.0f, "maximize"},
        {"damage_taken", 5.0f, 50.0f, 0.0f, 300.0f, "minimize"}
    };
    return config;
}

FitnessConfig createPuzzleGameFitness() {
    FitnessConfig config;
    config.baseFitness = 100.0f;
    config.aggregationMethod = "weighted_sum";
    config.metrics = {
        {"completion_time", 40.0f, 180.0f, 30.0f, 600.0f, "distance"},
        {"hints_used", 25.0f, 2.0f, 0.0f, 10.0f, "minimize"},
        {"attempts", 20.0f, 3.0f, 1.0f, 20.0f, "minimize"},
        {"player_satisfaction", 15.0f, 0.8f, 0.0f, 1.0f, "maximize"}
    };
    return config;
}

FitnessConfig createPlatformerGameFitness() {
    FitnessConfig config;
    config.baseFitness = 100.0f;
    config.aggregationMethod = "weighted_sum";
    config.metrics = {
        {"player_deaths", 30.0f, 0.25f, 0.0f, 5.0f, "distance"},
        {"completion_time", 25.0f, 300.0f, 60.0f, 900.0f, "distance"},
        {"collectibles_found", 20.0f, 0.8f, 0.0f, 1.0f, "maximize"},
        {"jump_accuracy", 15.0f, 0.85f, 0.0f, 1.0f, "distance"},
        {"backtracking", 10.0f, 0.1f, 0.0f, 1.0f, "minimize"}
    };
    return config;
}

FitnessConfig createRPGGameFitness() {
    FitnessConfig config;
    config.baseFitness = 100.0f;
    config.aggregationMethod = "weighted_sum";
    config.metrics = {
        {"quest_completion_rate", 30.0f, 0.9f, 0.0f, 1.0f, "maximize"},
        {"character_progression", 25.0f, 0.7f, 0.0f, 1.0f, "distance"},
        {"exploration_percentage", 20.0f, 0.6f, 0.0f, 1.0f, "maximize"},
        {"combat_efficiency", 15.0f, 0.75f, 0.0f, 1.0f, "distance"},
        {"resource_management", 10.0f, 0.8f, 0.0f, 1.0f, "distance"}
    };
    return config;
}

bool validateMetricsAvailable(const FitnessConfig& config, const MetricManager& metrics) {
    for (const auto& metricConfig : config.metrics) {
        if (!metrics.getComputedValue(metricConfig.metricName)) {
            std::cerr << "Warning: Metric '" << metricConfig.metricName 
                      << "' is not available in MetricManager" << std::endl;
            return false;
        }
    }
    return true;
}

} // namespace FitnessUtils