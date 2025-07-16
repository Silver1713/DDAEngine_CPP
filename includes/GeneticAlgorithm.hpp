#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <memory>
#include "DDAParameters.hpp"
#include "MetricManager.hpp"
#include "ParameterConfig.hpp"
#include "ConfigurableFitness.hpp"

struct Individual {
    bool evaluated = false;
    std::vector<float> genes;
    float fitness = 0.0f;
    
    Individual() = default;
    Individual(const std::vector<float>& g) : genes(g) {}
    
    bool operator<(const Individual& other) const {
        return fitness > other.fitness;
    }
};

// Configuration for genetic algorithm parameters
struct GAConfig {
    size_t populationSize = 20;
    size_t eliteSize = 5;
    float mutationRate = 0.1f;
    float crossoverRate = 0.7f;
    float mutationStrength = 0.2f;
    size_t tournamentSize = 3;
    std::string selectionMethod = "tournament"; // "tournament", "roulette", "rank"
    std::string crossoverMethod = "uniform"; // "uniform", "single_point", "multi_point"
    std::string mutationMethod = "gaussian"; // "gaussian", "uniform", "adaptive"
    
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;
};

class GeneticAlgorithm {
private:
    GAConfig config;
    size_t activeIndex = 0;
    size_t totalEvaluated = 0;
	std::vector<Individual> population;

    std::mt19937 rng;
    std::uniform_real_distribution<float> uniformDist;
    std::normal_distribution<float> normalDist;
    
    // Configuration and fitness function
    const ParameterConfig* parameterConfig;
    std::unique_ptr<ConfigurableFitness> fitnessEvaluator;
    DDAParameters templateParameters;
    
    // Legacy fitness function support
    std::function<float(const DDAParameters&, const MetricManager&)> legacyFitnessFunction;
    
public:
    // Constructors
    GeneticAlgorithm();
    explicit GeneticAlgorithm(const GAConfig& gaConfig);
    GeneticAlgorithm(const GAConfig& gaConfig, const ParameterConfig* paramConfig);
    
    // Configuration management
    void setConfig(const GAConfig& gaConfig);
    const GAConfig& getConfig() const { return config; }
    
    void setParameterConfig(const ParameterConfig* paramConfig);
    const ParameterConfig* getParameterConfig() const { return parameterConfig; }
    
    // Fitness function management
    void setFitnessEvaluator(std::unique_ptr<ConfigurableFitness> evaluator);
    void setFitnessFunction(std::function<float(const DDAParameters&, const MetricManager&)> func);
    void loadFitnessConfig(const std::string& filePath);
    
    // Population management
    void initializePopulation(const DDAParameters& baseParams);
    void evolve(const MetricManager& metrics);
    
    // Results
    DDAParameters getBestIndividual() const;
    std::vector<DDAParameters> getTopIndividuals(size_t count) const;
    
    // Statistics
    float getAverageFitness() const;
    float getBestFitness() const;
    float getWorstFitness() const;
    float getFitnessStandardDeviation() const;
    
    // Population diversity metrics
    float getPopulationDiversity() const;
    
    // Configuration loading
    bool loadConfigFromFile(const std::string& filePath);
    bool saveConfigToFile(const std::string& filePath) const;
    
    // Advanced features
    void injectIndividual(const DDAParameters& individual); // Add specific individual
    void setEliteIndividuals(const std::vector<DDAParameters>& elites); // Force specific elites
    
    // Debug and analysis
    std::string getEvolutionReport() const;
    std::vector<float> getFitnessHistory() const;


    Individual& GetCurrent();
    void SetActiveIndex(int index);
	int GetActiveIndex() const { return activeIndex; }

    float GetActiveFitness() const { return population[activeIndex].fitness; }
    float GetActiveFitness() {         return population[activeIndex].fitness;
	}

    bool AllEvaluated() const {
        return totalEvaluated >= config.populationSize;
	}

    DDAParameters GetActiveIndividualParams();

    void EvaluateFitnessIndividual(MetricManager& manager);

    void getUnevaluated();
private:
    // Core evolution operations
    void evaluateFitness(const MetricManager& metrics);
    std::vector<Individual> selection();
    Individual crossover(const Individual& parent1, const Individual& parent2);
    void mutate(Individual& individual);
    
    // Population initialization helpers
    Individual createRandomIndividual();
    Individual createIndividualFromParameters(const DDAParameters& params);
    DDAParameters createParametersFromIndividual(const Individual& individual) const;
    
    // Selection methods
    std::vector<Individual> tournamentSelection();
    std::vector<Individual> rouletteSelection();
    std::vector<Individual> rankSelection();
    
    // Crossover methods
    Individual uniformCrossover(const Individual& parent1, const Individual& parent2);
    Individual singlePointCrossover(const Individual& parent1, const Individual& parent2);
    Individual multiPointCrossover(const Individual& parent1, const Individual& parent2);
    
    // Mutation methods
    void gaussianMutation(Individual& individual);
    void uniformMutation(Individual& individual);
    void adaptiveMutation(Individual& individual);
    
    // Utility functions
    void validateConfiguration() const;
    void ensurePopulationSize();
    
    // Statistics tracking
    mutable std::vector<float> fitnessHistory;
    mutable std::vector<float> diversityHistory;
};