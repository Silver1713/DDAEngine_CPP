#include "DDAEngine.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

std::unique_ptr<DDAEngine> DDAEngine::instance = nullptr;

DDAEngine::DDAEngine() : metricManager(*MetricManager::getInstance()) {
    // Initialize GA with default configuration
    GAConfig gaConfig;
    gaConfig.populationSize = 50;
    gaConfig.eliteSize = 5;
    gaConfig.mutationRate = 0.1f;
    gaConfig.crossoverRate = 0.7f;
    geneticAlgorithm.setConfig(gaConfig);
    
    lastEvolutionTime = std::chrono::steady_clock::now();
}

DDAEngine& DDAEngine::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<DDAEngine>(new DDAEngine());
    }
    return *instance;
}

void DDAEngine::initialize() {
    geneticAlgorithm.setFitnessFunction(
        [this](const DDAParameters& params, const MetricManager& metrics) {
            return calculateFitness(params, metrics);
        }
    );

    geneticAlgorithm.initializePopulation(currentParameters);
    
    metricManager.addMetric("player_deaths", DDAMetricType::COUNT);
    metricManager.addMetric("completion_time", DDAMetricType::AVERAGE);
    metricManager.addMetric("accuracy", DDAMetricType::AVERAGE);
    metricManager.addMetric("enemies_killed", DDAMetricType::COUNT);
    metricManager.addMetric("damage_taken", DDAMetricType::SUM);
    metricManager.addMetric("powerups_collected", DDAMetricType::COUNT);
    metricManager.addMetric("distance_traveled", DDAMetricType::SUM);
}

void DDAEngine::collectLevelMetrics(const std::string& metricMatrix) {
    nlohmann::json matrix;
    try {
        matrix = nlohmann::json::parse(metricMatrix);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse metric matrix: " << e.what() << std::endl;
        return;
    }
    
    if (matrix.contains("player_deaths")) {
        metricManager.pushMetric("player_deaths", matrix["player_deaths"].get<int>());
    }
    if (matrix.contains("completion_time")) {
        metricManager.pushMetric("completion_time", matrix["completion_time"].get<float>());
    }
    if (matrix.contains("accuracy")) {
        metricManager.pushMetric("accuracy", matrix["accuracy"].get<float>());
    }
    if (matrix.contains("enemies_killed")) {
        metricManager.pushMetric("enemies_killed", matrix["enemies_killed"].get<int>());
    }
    if (matrix.contains("damage_taken")) {
        metricManager.pushMetric("damage_taken", matrix["damage_taken"].get<float>());
    }
    if (matrix.contains("powerups_collected")) {
        metricManager.pushMetric("powerups_collected", matrix["powerups_collected"].get<int>());
    }
    if (matrix.contains("distance_traveled")) {
        metricManager.pushMetric("distance_traveled", matrix["distance_traveled"].get<float>());
    }
    
    float skillLevel = calculatePlayerSkillLevel();
    playerPerformanceHistory.push_back({matrix.value("level_id", "unknown"), skillLevel});
    
    if (playerPerformanceHistory.size() > 100) {
        playerPerformanceHistory.erase(playerPerformanceHistory.begin());
    }
}

void DDAEngine::evolveParameters() {
    if (!isEvolutionEnabled || mode == DDA_MODE_FIXED) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastEvolution = std::chrono::duration_cast<std::chrono::minutes>(now - lastEvolutionTime);
    
    if (timeSinceLastEvolution < evolutionInterval && mode != DDA_MODE_LEARNING) {
        return;
    }
    
    geneticAlgorithm.evolve(metricManager);
    
    targetParameters = geneticAlgorithm.getBestIndividual();
    
    lastEvolutionTime = now;
    
    std::cout << "DDA Evolution complete. Best fitness: " << geneticAlgorithm.getBestFitness() 
              << ", Average fitness: " << geneticAlgorithm.getAverageFitness() << std::endl;
}

float DDAEngine::calculateFitness(const DDAParameters& params, const MetricManager& metrics) {
    float fitness = 100.0f;
    
    auto deathsOpt = metrics.getComputedValue("player_deaths");
    auto timeOpt = metrics.getComputedValue("completion_time");
    auto accuracyOpt = metrics.getComputedValue("accuracy");
    auto enemiesOpt = metrics.getComputedValue("enemies_killed");
    auto damageOpt = metrics.getComputedValue("damage_taken");
    
    if (!deathsOpt || !timeOpt || !accuracyOpt) {
        return 0.0f;
    }
    
    float deaths = 0.0f;
    float avgTime = 0.0f;
    float avgAccuracy = 0.0f;
    
    if (std::holds_alternative<int>(*deathsOpt)) {
        deaths = static_cast<float>(std::get<int>(*deathsOpt));
    } else if (std::holds_alternative<double>(*deathsOpt)) {
        deaths = static_cast<float>(std::get<double>(*deathsOpt));
    }
    
    if (std::holds_alternative<int>(*timeOpt)) {
        avgTime = static_cast<float>(std::get<int>(*timeOpt));
    } else if (std::holds_alternative<double>(*timeOpt)) {
        avgTime = static_cast<float>(std::get<double>(*timeOpt));
    }
    
    if (std::holds_alternative<int>(*accuracyOpt)) {
        avgAccuracy = static_cast<float>(std::get<int>(*accuracyOpt));
    } else if (std::holds_alternative<double>(*accuracyOpt)) {
        avgAccuracy = static_cast<float>(std::get<double>(*accuracyOpt));
    }
    
    float levelCount = std::max(1.0f, static_cast<float>(metrics.getMetricCount("completion_time")));
    float deathRate = deaths / levelCount;
    
    float deathPenalty = std::abs(deathRate - idealDeathRate) * 30.0f;
    fitness -= deathPenalty;
    
    float timePenalty = normalizeMetric(avgTime, 60.0f, 600.0f, idealCompletionTime) * 25.0f;
    fitness -= timePenalty;
    
    float accuracyPenalty = std::abs(avgAccuracy - idealAccuracy) * 20.0f;
    fitness -= accuracyPenalty;
    
    float difficultyBalance = 0.0f;
    if (deathRate > idealDeathRate * 1.5f) {
        difficultyBalance -= 10.0f;
    } else if (deathRate < idealDeathRate * 0.5f) {
        difficultyBalance -= 5.0f;
    }
    fitness += difficultyBalance;
    
    float parameterPenalty = 0.0f;
    float aggressiveness = params.getValue<float>("AI.aggressiveness", 0.5f);
    float enemyDensity = params.getValue<float>("PCG.enemyDensity", 0.5f);
    if (aggressiveness > 0.9f || aggressiveness < 0.1f) {
        parameterPenalty += 5.0f;
    }
    if (enemyDensity > 0.9f || enemyDensity < 0.1f) {
        parameterPenalty += 5.0f;
    }
    fitness -= parameterPenalty;
    
    return std::max(0.0f, fitness);
}

float DDAEngine::normalizeMetric(float value, float min, float max, float ideal) const {
    float normalized = (value - min) / (max - min);
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    float idealNormalized = (ideal - min) / (max - min);
    
    return std::abs(normalized - idealNormalized);
}

void DDAEngine::setParameters(const DDAParameters& params) {
    if (mode == DDA_MODE_ADAPTIVE) {
        targetParameters = params;
    } else {
        currentParameters = params;
        targetParameters = params;
    }
}

void DDAEngine::applyAdaptiveAdjustment(float deltaTime) {
    if (mode != DDA_MODE_ADAPTIVE) {
        return;
    }
    
    smoothParameterTransition(deltaTime);
}

void DDAEngine::smoothParameterTransition(float deltaTime) {
    float lerpSpeed = 0.5f;
    float t = lerpSpeed * deltaTime;
    t = std::min(1.0f, t);
    
    auto lerp = [](float a, float b, float t) {
        return a + (b - a) * t;
    };
    
    // Use the new interpolation method from DDAParameters
    currentParameters = DDAParameters::interpolate(currentParameters, targetParameters, t);
    
    currentParameters.clamp();
}

float DDAEngine::calculatePlayerSkillLevel() const {
    auto deathsOpt = metricManager.getComputedValue("player_deaths");
    auto timeOpt = metricManager.getComputedValue("completion_time");
    auto accuracyOpt = metricManager.getComputedValue("accuracy");
    auto enemiesOpt = metricManager.getComputedValue("enemies_killed");
    
    float skillLevel = 0.5f;
    
    if (timeOpt) {
        float avgTime = 0.0f;
        if (std::holds_alternative<int>(*timeOpt)) {
            avgTime = static_cast<float>(std::get<int>(*timeOpt));
        } else if (std::holds_alternative<double>(*timeOpt)) {
            avgTime = static_cast<float>(std::get<double>(*timeOpt));
        }
        float timeScore = 1.0f - normalizeMetric(avgTime, 60.0f, 600.0f, idealCompletionTime);
        skillLevel = skillLevel * 0.7f + timeScore * 0.3f;
    }
    
    if (accuracyOpt) {
        float accuracy = 0.0f;
        if (std::holds_alternative<int>(*accuracyOpt)) {
            accuracy = static_cast<float>(std::get<int>(*accuracyOpt));
        } else if (std::holds_alternative<double>(*accuracyOpt)) {
            accuracy = static_cast<float>(std::get<double>(*accuracyOpt));
        }
        skillLevel = skillLevel * 0.7f + accuracy * 0.3f;
    }
    
    if (deathsOpt) {
        float deaths = 0.0f;
        if (std::holds_alternative<int>(*deathsOpt)) {
            deaths = static_cast<float>(std::get<int>(*deathsOpt));
        } else if (std::holds_alternative<double>(*deathsOpt)) {
            deaths = static_cast<float>(std::get<double>(*deathsOpt));
        }
        float levelCount = std::max(1.0f, static_cast<float>(metricManager.getMetricCount("completion_time")));
        float deathRate = deaths / levelCount;
        float deathScore = 1.0f - std::min(1.0f, deathRate);
        skillLevel = skillLevel * 0.8f + deathScore * 0.2f;
    }
    
    return std::max(0.0f, std::min(1.0f, skillLevel));
}

nlohmann::json DDAEngine::getLevelGenerationHints() const {
    nlohmann::json hints;
    
    hints["enemy_spawn_rate"] = currentParameters.getValue<float>("PCG.enemyDensity", 0.5f);
    hints["min_enemies_per_room"] = currentParameters.getValue<int>("PCG.minEnemiesPerRoom", 1);
    hints["max_enemies_per_room"] = currentParameters.getValue<int>("PCG.maxEnemiesPerRoom", 5);
    hints["powerup_frequency"] = currentParameters.getValue<float>("PCG.powerUpFrequency", 0.3f);
    hints["obstacle_complexity"] = currentParameters.getValue<float>("PCG.obstacleComplexity", 0.5f);
    hints["path_branching"] = currentParameters.getValue<float>("PCG.pathBranching", 0.4f);
    hints["hazard_intensity"] = currentParameters.getValue<float>("PCG.hazardIntensity", 0.3f);
    
    float skillLevel = calculatePlayerSkillLevel();
    hints["recommended_difficulty"] = skillLevel;
    
    if (skillLevel < 0.3f) {
        hints["level_type"] = "tutorial";
    } else if (skillLevel < 0.6f) {
        hints["level_type"] = "standard";
    } else if (skillLevel < 0.85f) {
        hints["level_type"] = "challenging";
    } else {
        hints["level_type"] = "expert";
    }
    
    return hints;
}

std::string DDAEngine::exportParametersAsJson() const {
    nlohmann::json output;
    output["current_parameters"] = currentParameters.toJson();
    output["target_parameters"] = targetParameters.toJson();
    output["mode"] = static_cast<int>(mode);
    output["evolution_enabled"] = isEvolutionEnabled;
    output["player_skill_level"] = calculatePlayerSkillLevel();
    output["fitness_score"] = geneticAlgorithm.getBestFitness();
    output["generation_hints"] = getLevelGenerationHints();
    
    return output.dump(2);
}

void DDAEngine::importParametersFromJson(const std::string& json) {
    try {
        nlohmann::json input = nlohmann::json::parse(json);
        
        if (input.contains("current_parameters")) {
            currentParameters.fromJson(input["current_parameters"]);
        }
        if (input.contains("target_parameters")) {
            targetParameters.fromJson(input["target_parameters"]);
        }
        if (input.contains("mode")) {
            mode = static_cast<DDAMode>(input["mode"].get<int>());
        }
        if (input.contains("evolution_enabled")) {
            isEvolutionEnabled = input["evolution_enabled"].get<bool>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to import parameters: " << e.what() << std::endl;
    }
}

void DDAEngine::setIdealMetrics(float completionTime, float deathRate, float accuracy) {
    idealCompletionTime = completionTime;
    idealDeathRate = deathRate;
    idealAccuracy = accuracy;
}

void DDAEngine::resetToDefaults() {
    currentParameters = DDAParameters();
    targetParameters = DDAParameters();
    playerPerformanceHistory.clear();
    metricManager.clearAll();
    initialize();
}