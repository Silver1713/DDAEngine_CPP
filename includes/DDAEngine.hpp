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

// Use the DDAMode from DDAEngineAPI.h to avoid redefinition

class DDAEngine {
private:
    static std::unique_ptr<DDAEngine> instance;
    
    MetricManager& metricManager;
    GeneticAlgorithm geneticAlgorithm;
    DDAParameters currentParameters;
    DDAParameters targetParameters;
    
    DDAMode mode = DDA_MODE_ADAPTIVE;
    bool isEvolutionEnabled = true;
    
    std::vector<std::pair<std::string, float>> playerPerformanceHistory;
    
    std::chrono::steady_clock::time_point lastEvolutionTime;
    std::chrono::minutes evolutionInterval{5};
    
    float idealCompletionTime = 300.0f;  
    float idealDeathRate = 0.2f;        
    float idealAccuracy = 0.7f;         
    
    DDAEngine();
    
public:
    static DDAEngine& getInstance();
    
    void initialize();
    
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
    float calculateFitness(const DDAParameters& params, const MetricManager& metrics);
    
    void smoothParameterTransition(float deltaTime);
    
    float normalizeMetric(float value, float min, float max, float ideal) const;
};