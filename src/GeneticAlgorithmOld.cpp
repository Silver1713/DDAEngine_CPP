#include "GeneticAlgorithm.hpp"
#include <numeric>

void GeneticAlgorithm::initializePopulation(const DDAParameters& baseParams) {
    population.clear();
    population.reserve(populationSize);
    
    auto baseGenes = baseParams.toGeneticVector();
    size_t geneCount = baseGenes.size();
    
    population.push_back(Individual(baseGenes));
    
    for (size_t i = 1; i < populationSize; ++i) {
        auto individual = createRandomIndividual(geneCount);
        population.push_back(individual);
    }
}

Individual GeneticAlgorithm::createRandomIndividual(size_t geneCount) {
    Individual ind;
    ind.genes.reserve(geneCount);
    
    for (size_t i = 0; i < geneCount; ++i) {
        float value = uniformDist(rng);
        
        if (i == 1) { 
            value = 0.1f + value * 2.9f;
        } else if (i == 3) { 
            value = 0.1f + value * 2.9f;
        } else if (i == 4) { 
            value = 1.0f + value * 49.0f;
        } else if (i == 11 || i == 12) { 
            value = value; 
        } else if (i == 13) { 
            value = 0.1f + value * 2.9f;
        }
        
        ind.genes.push_back(value);
    }
    
    return ind;
}

void GeneticAlgorithm::evolve(const MetricManager& metrics) {
    evaluateFitness(metrics);
    
    std::sort(population.begin(), population.end());
    
    std::vector<Individual> newPopulation;
    newPopulation.reserve(populationSize);
    
    for (size_t i = 0; i < eliteSize && i < population.size(); ++i) {
        newPopulation.push_back(population[i]);
    }
    
    auto parents = selection();
    
    while (newPopulation.size() < populationSize) {
        size_t idx1 = std::uniform_int_distribution<size_t>(0, parents.size() - 1)(rng);
        size_t idx2 = std::uniform_int_distribution<size_t>(0, parents.size() - 1)(rng);
        
        if (uniformDist(rng) < crossoverRate) {
            auto child = crossover(parents[idx1], parents[idx2]);
            
            if (uniformDist(rng) < mutationRate) {
                mutate(child);
            }
            
            newPopulation.push_back(child);
        } else {
            newPopulation.push_back(parents[idx1]);
        }
    }
    
    population = std::move(newPopulation);
}

void GeneticAlgorithm::evaluateFitness(const MetricManager& metrics) {
    for (auto& individual : population) {
        DDAParameters params;
        params.fromGeneticVector(individual.genes);
        params.clamp();
        
        if (fitnessFunction) {
            individual.fitness = fitnessFunction(params, metrics);
        } else {
            individual.fitness = 0.0f;
        }
    }
}

std::vector<Individual> GeneticAlgorithm::selection() {
    std::vector<Individual> selected;
    size_t tournamentSize = 3;
    size_t selectCount = populationSize / 2;
    
    for (size_t i = 0; i < selectCount; ++i) {
        Individual* best = nullptr;
        
        for (size_t j = 0; j < tournamentSize; ++j) {
            size_t idx = std::uniform_int_distribution<size_t>(0, population.size() - 1)(rng);
            if (!best || population[idx].fitness > best->fitness) {
                best = &population[idx];
            }
        }
        
        if (best) {
            selected.push_back(*best);
        }
    }
    
    return selected;
}

Individual GeneticAlgorithm::crossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    child.genes.reserve(parent1.genes.size());
    
    for (size_t i = 0; i < parent1.genes.size(); ++i) {
        if (uniformDist(rng) < 0.5f) {
            child.genes.push_back(parent1.genes[i]);
        } else {
            child.genes.push_back(parent2.genes[i]);
        }
    }
    
    return child;
}

void GeneticAlgorithm::mutate(Individual& individual) {
    for (size_t i = 0; i < individual.genes.size(); ++i) {
        if (uniformDist(rng) < 0.1f) { 
            float delta = (uniformDist(rng) - 0.5f) * 0.2f;
            individual.genes[i] += delta;
            
            individual.genes[i] = std::max(0.0f, std::min(1.0f, individual.genes[i]));
            
            if (i == 1 || i == 3) {
                individual.genes[i] = std::max(0.1f, std::min(3.0f, individual.genes[i]));
            } else if (i == 4) {
                individual.genes[i] = std::max(1.0f, std::min(50.0f, individual.genes[i]));
            } else if (i == 13) {
                individual.genes[i] = std::max(0.1f, std::min(3.0f, individual.genes[i]));
            }
        }
    }
}

DDAParameters GeneticAlgorithm::getBestIndividual() const {
    if (population.empty()) {
        return DDAParameters();
    }
    
    DDAParameters params;
    params.fromGeneticVector(population[0].genes);
    params.clamp();
    return params;
}

std::vector<DDAParameters> GeneticAlgorithm::getTopIndividuals(size_t count) const {
    std::vector<DDAParameters> results;
    
    for (size_t i = 0; i < count && i < population.size(); ++i) {
        DDAParameters params;
        params.fromGeneticVector(population[i].genes);
        params.clamp();
        results.push_back(params);
    }
    
    return results;
}

float GeneticAlgorithm::getAverageFitness() const {
    if (population.empty()) return 0.0f;
    
    float sum = std::accumulate(population.begin(), population.end(), 0.0f,
                                [](float acc, const Individual& ind) {
                                    return acc + ind.fitness;
                                });
    return sum / population.size();
}

float GeneticAlgorithm::getBestFitness() const {
    if (population.empty()) return 0.0f;
    return population[0].fitness;
}