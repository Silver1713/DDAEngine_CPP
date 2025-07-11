#include <iostream>
#include <memory>
#include "ParameterConfig.hpp"
#include "ConfigurableFitness.hpp"
#include "GeneticAlgorithm.hpp"
#include "DDAParameters.hpp"
#include "MetricManager.hpp"

void demonstrateConfigurableSystem() {
    std::cout << "=== Configurable DDA System Demonstration ===" << std::endl;
    
    // 1. Load parameter configuration from JSON
    std::cout << "\n1. Loading parameter configuration..." << std::endl;
    ParameterConfig paramConfig;
    if (paramConfig.loadFromFile("config/shooter_game_config.json")) {
        std::cout << "✓ Parameter configuration loaded successfully!" << std::endl;
        std::cout << "Found " << paramConfig.getParameterGroups().size() << " parameter groups" << std::endl;
    } else {
        std::cout << "✗ Failed to load parameter configuration, using defaults" << std::endl;
        paramConfig = ParameterConfig::createDefaultConfig();
    }
    
    // 2. Create DDA parameters with configuration
    std::cout << "\n2. Creating DDA parameters..." << std::endl;
    DDAParameters ddaParams(&paramConfig);
    ddaParams.loadDefaults();
    
    // Show current parameter values
    std::cout << "Current parameter values:" << std::endl;
    for (const auto& group : paramConfig.getParameterGroups()) {
        std::cout << "  " << group.name << " Group:" << std::endl;
        auto groupParams = ddaParams.getParametersByGroup(group.name);
        for (const auto& [paramName, value] : groupParams) {
            std::visit([&](const auto& val) {
                std::cout << "    " << paramName << ": " << val << std::endl;
            }, value);
        }
    }
    
    // 3. Create configurable fitness function
    std::cout << "\n3. Setting up configurable fitness function..." << std::endl;
    auto fitnessEvaluator = std::make_unique<ConfigurableFitness>(paramConfig.getFitnessConfig());
    
    // 4. Setup genetic algorithm with configuration
    std::cout << "\n4. Configuring genetic algorithm..." << std::endl;
    GAConfig gaConfig;
    if (gaConfig.loadFromFile("config/genetic_algorithm.json")) {
        std::cout << "✓ GA configuration loaded from file" << std::endl;
    } else {
        std::cout << "Using default GA configuration" << std::endl;
    }
    
    GeneticAlgorithm ga(gaConfig, &paramConfig);
    ga.setFitnessEvaluator(std::move(fitnessEvaluator));
    
    // 5. Initialize population
    std::cout << "\n5. Initializing population..." << std::endl;
    ga.initializePopulation(ddaParams);
    std::cout << "Population size: " << gaConfig.populationSize << std::endl;
    
    // 6. Setup metric manager and simulate some data
    std::cout << "\n6. Simulating gameplay metrics..." << std::endl;
    MetricManager& metrics = *MetricManager::getInstance();
    
    // Add metrics that match our fitness configuration
    metrics.addMetric("player_deaths", DDAMetricType::COUNT);
    metrics.addMetric("completion_time", DDAMetricType::AVERAGE);
    metrics.addMetric("accuracy", DDAMetricType::AVERAGE);
    metrics.addMetric("enemies_killed", DDAMetricType::COUNT);
    metrics.addMetric("damage_taken", DDAMetricType::SUM);
    
    // Simulate gameplay sessions
    for (int session = 1; session <= 5; ++session) {
        std::cout << "  Session " << session << ": ";
        
        // Simulate metrics (in a real game, these would come from actual gameplay)
        float sessionDifficulty = 0.3f + (session * 0.15f); // Gradually increase difficulty
        
        int deaths = static_cast<int>(sessionDifficulty * 3);
        float completionTime = 180.0f + (sessionDifficulty * 120.0f);
        float accuracy = 0.8f - (sessionDifficulty * 0.2f);
        int enemiesKilled = 15 + static_cast<int>(sessionDifficulty * 20);
        float damageTaken = 30.0f + (sessionDifficulty * 50.0f);
        
        metrics.pushMetric("player_deaths", deaths);
        metrics.pushMetric("completion_time", completionTime);
        metrics.pushMetric("accuracy", accuracy);
        metrics.pushMetric("enemies_killed", enemiesKilled);
        metrics.pushMetric("damage_taken", damageTaken);
        
        std::cout << "Deaths: " << deaths << ", Time: " << completionTime 
                  << "s, Accuracy: " << (accuracy * 100) << "%" << std::endl;
    }
    
    // 7. Evolve parameters
    std::cout << "\n7. Evolving parameters..." << std::endl;
    for (int generation = 1; generation <= 3; ++generation) {
        std::cout << "Generation " << generation << ": ";
        ga.evolve(metrics);
        std::cout << "Best fitness: " << ga.getBestFitness() 
                  << ", Avg fitness: " << ga.getAverageFitness() << std::endl;
    }
    
    // 8. Get evolved parameters
    std::cout << "\n8. Results after evolution:" << std::endl;
    DDAParameters evolvedParams = ga.getBestIndividual();
    
    std::cout << "Best evolved parameters:" << std::endl;
    for (const auto& group : paramConfig.getParameterGroups()) {
        std::cout << "  " << group.name << " Group:" << std::endl;
        auto groupParams = evolvedParams.getParametersByGroup(group.name);
        for (const auto& [paramName, value] : groupParams) {
            std::visit([&](const auto& val) {
                std::cout << "    " << paramName << ": " << val << std::endl;
            }, value);
        }
    }
    
    // 9. Show evolution report
    std::cout << "\n9. Evolution Report:" << std::endl;
    std::cout << ga.getEvolutionReport() << std::endl;
    
    // 10. Export configuration
    std::cout << "\n10. Exporting evolved configuration..." << std::endl;
    std::string evolvedJson = evolvedParams.toJson().dump(2);
    std::cout << "Evolved parameters (JSON):" << std::endl;
    std::cout << evolvedJson << std::endl;
    
    std::cout << "\n=== Demonstration Complete ===" << std::endl;
    std::cout << "The system is now fully configurable through JSON files!" << std::endl;
    std::cout << "✓ Parameters are user-defined via JSON configuration" << std::endl;
    std::cout << "✓ Fitness function is configurable with different evaluation types" << std::endl;
    std::cout << "✓ Genetic algorithm settings are externally configurable" << std::endl;
    std::cout << "✓ Multiple selection, crossover, and mutation methods available" << std::endl;
    std::cout << "✓ Backward compatibility maintained with legacy APIs" << std::endl;
}

void demonstrateParameterInterpolation() {
    std::cout << "\n=== Parameter Interpolation Demonstration ===" << std::endl;
    
    ParameterConfig config = ParameterConfig::createDefaultConfig();
    
    // Create two different parameter sets
    DDAParameters easyParams(&config);
    easyParams.setValue("AI.aggressiveness", 0.3f);
    easyParams.setValue("AI.accuracy", 0.5f);
    easyParams.setValue("PCG.enemyDensity", 0.3f);
    
    DDAParameters hardParams(&config);
    hardParams.setValue("AI.aggressiveness", 0.9f);
    hardParams.setValue("AI.accuracy", 0.9f);
    hardParams.setValue("PCG.enemyDensity", 0.8f);
    
    std::cout << "Easy parameters - Aggressiveness: " << easyParams.getValue<float>("AI.aggressiveness") << std::endl;
    std::cout << "Hard parameters - Aggressiveness: " << hardParams.getValue<float>("AI.aggressiveness") << std::endl;
    
    // Interpolate between them
    for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
        DDAParameters interpolated = DDAParameters::interpolate(easyParams, hardParams, t);
        std::cout << "t=" << t << " - Aggressiveness: " 
                  << interpolated.getValue<float>("AI.aggressiveness") << std::endl;
    }
}

void demonstrateCustomGameConfigs() {
    std::cout << "\n=== Custom Game Configuration Examples ===" << std::endl;
    
    // Show different fitness configurations for different game types
    auto shooterFitness = FitnessUtils::createShooterGameFitness();
    auto puzzleFitness = FitnessUtils::createPuzzleGameFitness();
    auto platformerFitness = FitnessUtils::createPlatformerGameFitness();
    auto rpgFitness = FitnessUtils::createRPGGameFitness();
    
    std::cout << "Shooter game fitness metrics:" << std::endl;
    for (const auto& metric : shooterFitness.metrics) {
        std::cout << "  " << metric.metricName << " (weight: " << metric.weight 
                  << ", ideal: " << metric.idealValue << ")" << std::endl;
    }
    
    std::cout << "\nPuzzle game fitness metrics:" << std::endl;
    for (const auto& metric : puzzleFitness.metrics) {
        std::cout << "  " << metric.metricName << " (weight: " << metric.weight 
                  << ", ideal: " << metric.idealValue << ")" << std::endl;
    }
}

int main() {
    try {
        demonstrateConfigurableSystem();
        demonstrateParameterInterpolation();
        demonstrateCustomGameConfigs();
        
        std::cout << "\n🎉 All demonstrations completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}