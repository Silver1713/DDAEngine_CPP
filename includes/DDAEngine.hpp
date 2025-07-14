#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <chrono>
#include "MetricManager.hpp"
#include "GeneticAlgorithm.hpp"
#include "DDAParameters.hpp"
#include "DDAEngineAPI.h"
#include "ConfigurableFitness.hpp"
#include "ParameterConfig.hpp"

// Use the DDAMode from DDAEngineAPI.h to avoid redefinition

class DDAEngine {
private:
    static std::unique_ptr<DDAEngine> instance;

    std::unique_ptr<ParameterConfig> configuration;
    std::unique_ptr<ConfigurableFitness> fitnessEvaluator;
    
    MetricManager& metricManager;
    GeneticAlgorithm geneticAlgorithm;
    DDAParameters currentParameters;
    DDAParameters targetParameters;
    
    DDAMode mode = DDA_MODE_ADAPTIVE;
    bool isEvolutionEnabled = true;
    bool isInitialized = false;
    
    std::vector<std::pair<std::string, float>> playerPerformanceHistory;
    
    std::chrono::steady_clock::time_point lastEvolutionTime;
    std::chrono::minutes evolutionInterval{5};
    
    // Legacy support
    float idealCompletionTime = 300.0f;  
    float idealDeathRate = 0.2f;        
    float idealAccuracy = 0.7f;         
    
    DDAEngine();
    
public:
    static DDAEngine& getInstance();
    
    // Configuration-based initialization
    void initialize();
    void initialize(std::unique_ptr<ParameterConfig> config);
    void initialize(std::unique_ptr<ParameterConfig> config, std::unique_ptr<ConfigurableFitness> fitness);
    
    // Configuration management
    void loadConfiguration(const std::string& configPath);
    void setConfiguration(std::unique_ptr<ParameterConfig> config);
    ParameterConfig* getConfiguration() { return configuration.get(); }
    const ParameterConfig* getConfiguration() const { return configuration.get(); }
    
    // Fitness management
    void setFitnessEvaluator(std::unique_ptr<ConfigurableFitness> fitness);
    ConfigurableFitness* getFitnessEvaluator() { return fitnessEvaluator.get(); }
    
    // Genetic algorithm configuration
    void setGAConfig(const GAConfig& config);
    GeneticAlgorithm& getGeneticAlgorithm() { return geneticAlgorithm; }
    
    void collectLevelMetrics(const std::string& metricMatrix);
    
    void evolveParameters();
    
    DDAParameters getCurrentParameters() const { return currentParameters; }
    
    void setParameters(const DDAParameters& params);
    
    void setMode(DDAMode newMode) { mode = newMode; }
    DDAMode getMode() const { return mode; }
    
    void setEvolutionEnabled(bool enabled) { isEvolutionEnabled = enabled; }
    bool isEvolutionEnabledStatus() const { return isEvolutionEnabled; }
    
    void setIdealMetrics(float completionTime, float deathRate, float accuracy);
    
    std::string exportParametersAsJson() const;
    void importParametersFromJson(const std::string& json);
    
    float calculatePlayerSkillLevel() const;
    
    void applyAdaptiveAdjustment(float deltaTime);
    
    nlohmann::json getLevelGenerationHints() const;
    
    void resetToDefaults();
    
private:
    // Legacy fitness calculation for backward compatibility
    float calculateFitness(const DDAParameters& params, const MetricManager& metrics);
    
    void smoothParameterTransition(float deltaTime);
    
    float normalizeMetric(float value, float min, float max, float ideal) const;
    
    // Helper to ensure configuration exists
    void ensureConfiguration();
};