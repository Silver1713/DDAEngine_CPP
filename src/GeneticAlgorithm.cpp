#include "GeneticAlgorithm.hpp"
#include <numeric>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>

// GAConfig implementation
nlohmann::json GAConfig::toJson() const {
    return nlohmann::json{
        {"populationSize", populationSize},
        {"eliteSize", eliteSize},
        {"mutationRate", mutationRate},
        {"crossoverRate", crossoverRate},
        {"mutationStrength", mutationStrength},
        {"tournamentSize", tournamentSize},
        {"selectionMethod", selectionMethod},
        {"crossoverMethod", crossoverMethod},
        {"mutationMethod", mutationMethod}
    };
}

void GAConfig::fromJson(const nlohmann::json& j) {
    populationSize = j.value("populationSize", populationSize);
    eliteSize = j.value("eliteSize", eliteSize);
    mutationRate = j.value("mutationRate", mutationRate);
    crossoverRate = j.value("crossoverRate", crossoverRate);
    mutationStrength = j.value("mutationStrength", mutationStrength);
    tournamentSize = j.value("tournamentSize", tournamentSize);
    selectionMethod = j.value("selectionMethod", selectionMethod);
    crossoverMethod = j.value("crossoverMethod", crossoverMethod);
    mutationMethod = j.value("mutationMethod", mutationMethod);
}

bool GAConfig::loadFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        
        nlohmann::json j;
        file >> j;
        fromJson(j);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading GA config: " << e.what() << std::endl;
        return false;
    }
}

bool GAConfig::saveToFile(const std::string& filePath) const {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        
        file << toJson().dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving GA config: " << e.what() << std::endl;
        return false;
    }
}

// GeneticAlgorithm implementation
GeneticAlgorithm::GeneticAlgorithm() 
    : parameterConfig(nullptr), uniformDist(0.0f, 1.0f), normalDist(0.0f, 1.0f) {
    rng.seed(std::random_device{}());
}

GeneticAlgorithm::GeneticAlgorithm(const GAConfig& gaConfig) 
    : config(gaConfig), parameterConfig(nullptr), uniformDist(0.0f, 1.0f), normalDist(0.0f, 1.0f) {
    rng.seed(std::random_device{}());
    validateConfiguration();
}

GeneticAlgorithm::GeneticAlgorithm(const GAConfig& gaConfig, const ParameterConfig* paramConfig)
    : config(gaConfig), parameterConfig(paramConfig), templateParameters(paramConfig),
      uniformDist(0.0f, 1.0f), normalDist(0.0f, 1.0f) {
    rng.seed(std::random_device{}());
    validateConfiguration();
}

void GeneticAlgorithm::setConfig(const GAConfig& gaConfig) {
    config = gaConfig;
    validateConfiguration();
    ensurePopulationSize();
}

void GeneticAlgorithm::setParameterConfig(const ParameterConfig* paramConfig) {
    parameterConfig = paramConfig;
    templateParameters.setConfig(paramConfig);
    templateParameters.loadDefaults();
}

void GeneticAlgorithm::setFitnessEvaluator(std::unique_ptr<ConfigurableFitness> evaluator) {
    fitnessEvaluator = std::move(evaluator);
    legacyFitnessFunction = nullptr; // Clear legacy function
}

void GeneticAlgorithm::setFitnessFunction(std::function<float(const DDAParameters&, const MetricManager&)> func) {
    legacyFitnessFunction = func;
    fitnessEvaluator.reset(); // Clear configurable evaluator
}

void GeneticAlgorithm::loadFitnessConfig(const std::string& filePath) {
    auto fitness = std::make_unique<ConfigurableFitness>();
    if (fitness->loadFromFile(filePath)) {
        setFitnessEvaluator(std::move(fitness));
    } else {
        std::cerr << "Failed to load fitness config from: " << filePath << std::endl;
    }
}

void GeneticAlgorithm::initializePopulation(const DDAParameters& baseParams) {
    templateParameters = baseParams;
    population.clear();
    population.reserve(config.populationSize);
    
    // Add the base parameters as first individual
    population.push_back(createIndividualFromParameters(baseParams));
    
    // Generate random individuals
    for (size_t i = 1; i < config.populationSize; ++i) {
        population.push_back(createRandomIndividual());
    }
    
    fitnessHistory.clear();
    diversityHistory.clear();
}

void GeneticAlgorithm::evolve(const MetricManager& metrics) {
   // evaluateFitness(metrics);

    if (!AllEvaluated())
		throw "Not all individuals have been evaluated yet.";

	
    // Sort population by fitness (descending)
    std::sort(population.begin(), population.end());
    
    // Track statistics
    fitnessHistory.push_back(getBestFitness());
    diversityHistory.push_back(getPopulationDiversity());
    
    std::vector<Individual> newPopulation;
    newPopulation.reserve(config.populationSize);
    
    // Preserve elite individuals
    for (size_t i = 0; i < config.eliteSize && i < population.size(); ++i) {
        newPopulation.push_back(population[i]);
    }
    
    // Generate offspring
    auto parents = selection();
    while (newPopulation.size() < config.populationSize) {
        size_t idx1 = std::uniform_int_distribution<size_t>(0, parents.size() - 1)(rng);
        size_t idx2 = std::uniform_int_distribution<size_t>(0, parents.size() - 1)(rng);
        
        Individual child;
        if (uniformDist(rng) < config.crossoverRate) {
            child = crossover(parents[idx1], parents[idx2]);
        } else {
            child = parents[idx1];
        }
        
        if (uniformDist(rng) < config.mutationRate) {
            mutate(child);
        }
        
        newPopulation.push_back(child);


        totalEvaluated--;

        
    }
    
    population = std::move(newPopulation);
}

DDAParameters GeneticAlgorithm::getBestIndividual() const {
    if (population.empty()) {
        return templateParameters;
    }
    return createParametersFromIndividual(population[0]);
}

std::vector<DDAParameters> GeneticAlgorithm::getTopIndividuals(size_t count) const {
    std::vector<DDAParameters> results;
    results.reserve(count);
    
    for (size_t i = 0; i < count && i < population.size(); ++i) {
        results.push_back(createParametersFromIndividual(population[i]));
    }
    
    return results;
}

float GeneticAlgorithm::getAverageFitness() const {
    if (population.empty()) return 0.0f;
    
    float sum = std::accumulate(population.begin(), population.end(), 0.0f,
        [](float acc, const Individual& ind) { return acc + ind.fitness; });
    return sum / population.size();
}

float GeneticAlgorithm::getBestFitness() const {
    if (population.empty()) return 0.0f;
    return population[0].fitness;
}

float GeneticAlgorithm::getWorstFitness() const {
    if (population.empty()) return 0.0f;
    return population.back().fitness;
}

float GeneticAlgorithm::getFitnessStandardDeviation() const {
    if (population.size() < 2) return 0.0f;
    
    float mean = getAverageFitness();
    float variance = 0.0f;
    
    for (const auto& individual : population) {
        float diff = individual.fitness - mean;
        variance += diff * diff;
    }
    
    variance /= population.size();
    return std::sqrt(variance);
}

float GeneticAlgorithm::getPopulationDiversity() const {
    if (population.size() < 2) return 0.0f;
    
    float totalDistance = 0.0f;
    size_t comparisons = 0;
    
    for (size_t i = 0; i < population.size(); ++i) {
        for (size_t j = i + 1; j < population.size(); ++j) {
            float distance = 0.0f;
            const auto& genes1 = population[i].genes;
            const auto& genes2 = population[j].genes;
            
            for (size_t k = 0; k < genes1.size() && k < genes2.size(); ++k) {
                float diff = genes1[k] - genes2[k];
                distance += diff * diff;
            }
            
            totalDistance += std::sqrt(distance);
            comparisons++;
        }
    }
    
    return comparisons > 0 ? totalDistance / comparisons : 0.0f;
}

bool GeneticAlgorithm::loadConfigFromFile(const std::string& filePath) {
    return config.loadFromFile(filePath);
}

bool GeneticAlgorithm::saveConfigToFile(const std::string& filePath) const {
    return config.saveToFile(filePath);
}

void GeneticAlgorithm::injectIndividual(const DDAParameters& individual) {
    if (population.size() >= config.populationSize) {
        // Replace worst individual
        population.back() = createIndividualFromParameters(individual);
    } else {
        population.push_back(createIndividualFromParameters(individual));
    }
}

void GeneticAlgorithm::setEliteIndividuals(const std::vector<DDAParameters>& elites) {
    size_t eliteCount = std::min(elites.size(), config.eliteSize);
    for (size_t i = 0; i < eliteCount && i < population.size(); ++i) {
        population[i] = createIndividualFromParameters(elites[i]);
    }
}

std::string GeneticAlgorithm::getEvolutionReport() const {
    std::stringstream ss;
    ss << "=== Genetic Algorithm Evolution Report ===\n";
    ss << "Population Size: " << population.size() << "\n";
    ss << "Best Fitness: " << getBestFitness() << "\n";
    ss << "Average Fitness: " << getAverageFitness() << "\n";
    ss << "Worst Fitness: " << getWorstFitness() << "\n";
    ss << "Fitness Std Dev: " << getFitnessStandardDeviation() << "\n";
    ss << "Population Diversity: " << getPopulationDiversity() << "\n";
    ss << "Generations: " << fitnessHistory.size() << "\n";
    
    if (fitnessHistory.size() > 1) {
        ss << "Fitness Improvement: " << (getBestFitness() - fitnessHistory[0]) << "\n";
    }
    
    return ss.str();
}

std::vector<float> GeneticAlgorithm::getFitnessHistory() const {
    return fitnessHistory;
}

Individual& GeneticAlgorithm::GetCurrent()
{
	if (population.size() > 0) {
        return population[activeIndex];
    }
    else {
        throw std::runtime_error("Population is empty, cannot get current individual.");
	}
}



void GeneticAlgorithm::EvaluateFitnessIndividual(MetricManager& manager)
{
	Individual& individual = GetCurrent();
    if (individual.evaluated)
    {
		std::cout << "Individual already evaluated." << std::endl;
        return; // Already evaluated
	}

	evaluateFitness(manager);
}


void GeneticAlgorithm::getUnevaluated()
{
    if (AllEvaluated()) return;

    auto it = std::find_if(population.begin(), population.end(), [](Individual const& individual)
    {
        return !individual.evaluated;
    });

    activeIndex = it - std::begin(population);
}


DDAParameters GeneticAlgorithm::GetActiveIndividualParams()
{
	Individual& individual = GetCurrent();
	return createParametersFromIndividual(individual);
}


void GeneticAlgorithm::SetActiveIndex(int index)
{
    if (index >= 0 && index < population.size()) {
        activeIndex = index;
    }
    else {
        throw std::out_of_range("Invalid index");
    }
}



// Private implementation methods
void GeneticAlgorithm::evaluateFitness(const MetricManager& metrics) {
   /* for (auto& individual : population) {*/
    Individual& individual = population[activeIndex];
        if (individual.evaluated)
        {
            return;
        };
        DDAParameters params = createParametersFromIndividual(individual);
        
        if (fitnessEvaluator) {
            individual.fitness = fitnessEvaluator->evaluateFitness(params.getParameters(), metrics);
        } else if (legacyFitnessFunction) {
            individual.fitness = legacyFitnessFunction(params, metrics);
        } else {
            individual.fitness = 0.0f;
        }
        totalEvaluated++;
        individual.evaluated = true;
    //}
}

std::vector<Individual> GeneticAlgorithm::selection() {
    if (config.selectionMethod == "tournament") {
        return tournamentSelection();
    } else if (config.selectionMethod == "roulette") {
        return rouletteSelection();
    } else if (config.selectionMethod == "rank") {
        return rankSelection();
    } else {
        return tournamentSelection(); // Default
    }
}

Individual GeneticAlgorithm::crossover(const Individual& parent1, const Individual& parent2) {
    if (config.crossoverMethod == "uniform") {
        return uniformCrossover(parent1, parent2);
    } else if (config.crossoverMethod == "single_point") {
        return singlePointCrossover(parent1, parent2);
    } else if (config.crossoverMethod == "multi_point") {
        return multiPointCrossover(parent1, parent2);
    } else {
        return uniformCrossover(parent1, parent2); // Default
    }
}

void GeneticAlgorithm::mutate(Individual& individual) {
    if (config.mutationMethod == "gaussian") {
        gaussianMutation(individual);
    } else if (config.mutationMethod == "uniform") {
        uniformMutation(individual);
    } else if (config.mutationMethod == "adaptive") {
        adaptiveMutation(individual);
    } else {
        gaussianMutation(individual); // Default
    }
}

Individual GeneticAlgorithm::createRandomIndividual() {
    if (parameterConfig) {
        // Create random individual based on parameter configuration
        DDAParameters randomParams(parameterConfig);
        randomParams.loadDefaults();
        
        // Randomize values within bounds
        for (const auto& group : parameterConfig->getParameterGroups()) {
            for (const auto& param : group.parameters) {
                std::string key = group.name + "." + param.name;
                
                if (param.type == "float") {
                    float minVal = std::get<float>(param.minValue);
                    float maxVal = std::get<float>(param.maxValue);
                    float randomVal = minVal + uniformDist(rng) * (maxVal - minVal);
                    randomParams.setValue(key, randomVal);
                } else if (param.type == "int") {
                    int minVal = std::get<int>(param.minValue);
                    int maxVal = std::get<int>(param.maxValue);
                    int randomVal = minVal + static_cast<int>(uniformDist(rng) * (maxVal - minVal));
                    randomParams.setValue(key, randomVal);
                } else if (param.type == "bool") {
                    bool randomVal = uniformDist(rng) > 0.5f;
                    randomParams.setValue(key, randomVal);
                }
            }
        }
        
        return createIndividualFromParameters(randomParams);
    } else {
        // Legacy random individual creation
        auto baseGenes = templateParameters.toGeneticVector();
        Individual ind;
        ind.genes.reserve(baseGenes.size());
        
        for (size_t i = 0; i < baseGenes.size(); ++i) {
            ind.genes.push_back(uniformDist(rng));
        }
        
        return ind;
    }
}

Individual GeneticAlgorithm::createIndividualFromParameters(const DDAParameters& params) {
    return Individual(params.toGeneticVector());
}

DDAParameters GeneticAlgorithm::createParametersFromIndividual(const Individual& individual) const {
    DDAParameters params = templateParameters;
    params.fromGeneticVector(individual.genes);
    params.clamp();
    return params;
}

// Selection methods implementation
std::vector<Individual> GeneticAlgorithm::tournamentSelection() {
    std::vector<Individual> selected;
    size_t selectCount = config.populationSize / 2;
    
    for (size_t i = 0; i < selectCount; ++i) {
        Individual* best = nullptr;
        
        for (size_t j = 0; j < config.tournamentSize; ++j) {
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

std::vector<Individual> GeneticAlgorithm::rouletteSelection() {
    std::vector<Individual> selected;
    size_t selectCount = config.populationSize / 2;
    
    // Calculate total fitness
    float totalFitness = 0.0f;
    float minFitness = population.empty() ? 0.0f : population.back().fitness;
    
    for (const auto& individual : population) {
        totalFitness += std::max(0.0f, individual.fitness - minFitness + 1.0f);
    }
    
    if (totalFitness <= 0.0f) {
        // Fallback to tournament if fitness is problematic
        return tournamentSelection();
    }
    
    for (size_t i = 0; i < selectCount; ++i) {
        float r = uniformDist(rng) * totalFitness;
        float sum = 0.0f;
        
        for (const auto& individual : population) {
            sum += std::max(0.0f, individual.fitness - minFitness + 1.0f);
            if (sum >= r) {
                selected.push_back(individual);
                break;
            }
        }
    }
    
    return selected;
}

std::vector<Individual> GeneticAlgorithm::rankSelection() {
    std::vector<Individual> selected;
    size_t selectCount = config.populationSize / 2;
    
    // Population is already sorted by fitness
    size_t totalRank = population.size() * (population.size() + 1) / 2;
    
    for (size_t i = 0; i < selectCount; ++i) {
        float r = uniformDist(rng) * totalRank;
        float sum = 0.0f;
        
        for (size_t j = 0; j < population.size(); ++j) {
            sum += population.size() - j; // Higher rank for better fitness
            if (sum >= r) {
                selected.push_back(population[j]);
                break;
            }
        }
    }
    
    return selected;
}

// Crossover methods implementation
Individual GeneticAlgorithm::uniformCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    size_t maxSize = std::max(parent1.genes.size(), parent2.genes.size());
    child.genes.reserve(maxSize);
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (uniformDist(rng) < 0.5f && i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        } else if (i < parent2.genes.size()) {
            child.genes.push_back(parent2.genes[i]);
        } else if (i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        }
    }
    
    return child;
}

Individual GeneticAlgorithm::singlePointCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    size_t maxSize = std::max(parent1.genes.size(), parent2.genes.size());
    child.genes.reserve(maxSize);
    
    size_t crossoverPoint = std::uniform_int_distribution<size_t>(1, maxSize - 1)(rng);
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (i < crossoverPoint && i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        } else if (i < parent2.genes.size()) {
            child.genes.push_back(parent2.genes[i]);
        } else if (i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        }
    }
    
    return child;
}

Individual GeneticAlgorithm::multiPointCrossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    size_t maxSize = std::max(parent1.genes.size(), parent2.genes.size());
    child.genes.reserve(maxSize);
    
    // Create random crossover points
    std::vector<bool> useParent1(maxSize);
    bool currentParent = uniformDist(rng) < 0.5f;
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (uniformDist(rng) < 0.1f) { // 10% chance to switch parent
            currentParent = !currentParent;
        }
        useParent1[i] = currentParent;
    }
    
    for (size_t i = 0; i < maxSize; ++i) {
        if (useParent1[i] && i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        } else if (!useParent1[i] && i < parent2.genes.size()) {
            child.genes.push_back(parent2.genes[i]);
        } else if (i < parent1.genes.size()) {
            child.genes.push_back(parent1.genes[i]);
        } else if (i < parent2.genes.size()) {
            child.genes.push_back(parent2.genes[i]);
        }
    }
    
    return child;
}

// Mutation methods implementation
void GeneticAlgorithm::gaussianMutation(Individual& individual) {
    for (float& gene : individual.genes) {
        if (uniformDist(rng) < 0.1f) { // 10% chance per gene
            float delta = normalDist(rng) * config.mutationStrength;
            gene += delta;
            gene = std::max(0.0f, std::min(1.0f, gene));
        }
    }
}

void GeneticAlgorithm::uniformMutation(Individual& individual) {
    for (float& gene : individual.genes) {
        if (uniformDist(rng) < 0.1f) { // 10% chance per gene
            float delta = (uniformDist(rng) - 0.5f) * config.mutationStrength * 2.0f;
            gene += delta;
            gene = std::max(0.0f, std::min(1.0f, gene));
        }
    }
}

void GeneticAlgorithm::adaptiveMutation(Individual& individual) {
    // Adaptive mutation based on population diversity
    float diversity = getPopulationDiversity();
    float adaptiveStrength = config.mutationStrength * (1.0f + (1.0f - diversity));
    
    for (float& gene : individual.genes) {
        if (uniformDist(rng) < 0.1f) { // 10% chance per gene
            float delta = normalDist(rng) * adaptiveStrength;
            gene += delta;
            gene = std::max(0.0f, std::min(1.0f, gene));
        }
    }
}

void GeneticAlgorithm::validateConfiguration() const {
    if (config.populationSize < 2) {
        throw std::invalid_argument("Population size must be at least 2");
    }
    if (config.eliteSize >= config.populationSize) {
        throw std::invalid_argument("Elite size must be less than population size");
    }
    if (config.mutationRate < 0.0f || config.mutationRate > 1.0f) {
        throw std::invalid_argument("Mutation rate must be between 0 and 1");
    }
    if (config.crossoverRate < 0.0f || config.crossoverRate > 1.0f) {
        throw std::invalid_argument("Crossover rate must be between 0 and 1");
    }
}

void GeneticAlgorithm::ensurePopulationSize() {
    if (population.size() > config.populationSize) {
        population.resize(config.populationSize);
    }
}