#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include "DDAParameters.hpp"
#include "MetricManager.hpp"

struct Individual {
    std::vector<float> genes;
    float fitness = 0.0f;
    
    Individual() = default;
    Individual(const std::vector<float>& g) : genes(g) {}
    
    bool operator<(const Individual& other) const {
        return fitness > other.fitness;
    }
};

class GeneticAlgorithm {
private:
    size_t populationSize;
    size_t eliteSize;
    float mutationRate;
    float crossoverRate;
    std::vector<Individual> population;
    std::mt19937 rng;
    std::uniform_real_distribution<float> uniformDist;
    
    std::function<float(const DDAParameters&, const MetricManager&)> fitnessFunction;
    
public:
    GeneticAlgorithm(size_t popSize = 50, size_t elite = 5, 
                     float mutRate = 0.1f, float crossRate = 0.7f)
        : populationSize(popSize), eliteSize(elite), 
          mutationRate(mutRate), crossoverRate(crossRate),
          rng(std::random_device{}()), uniformDist(0.0f, 1.0f) {}
    
    void initializePopulation(const DDAParameters& baseParams);
    
    void evolve(const MetricManager& metrics);
    
    DDAParameters getBestIndividual() const;
    
    std::vector<DDAParameters> getTopIndividuals(size_t count) const;
    
    void setFitnessFunction(std::function<float(const DDAParameters&, const MetricManager&)> func) {
        fitnessFunction = func;
    }
    
    float getAverageFitness() const;
    float getBestFitness() const;
    
private:
    void evaluateFitness(const MetricManager& metrics);
    
    std::vector<Individual> selection();
    
    Individual crossover(const Individual& parent1, const Individual& parent2);
    
    void mutate(Individual& individual);
    
    Individual createRandomIndividual(size_t geneCount);
};