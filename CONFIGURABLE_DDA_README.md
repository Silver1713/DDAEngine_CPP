# Configurable DDA System

This document describes the new configurable Dynamic Difficulty Adjustment (DDA) system that replaces hardcoded parameters with user-defined JSON configurations.

## Overview

The enhanced DDA system provides:
- **User-defined parameters** through JSON configuration files
- **Configurable fitness functions** with multiple evaluation strategies  
- **Flexible genetic algorithm** settings
- **Backward compatibility** with existing APIs
- **Multiple game type presets** (shooter, puzzle, platformer, RPG)

## Key Components

### 1. Parameter Configuration System

#### ParameterConfig Class
Manages parameter definitions and constraints:
```cpp
#include "ParameterConfig.hpp"

// Load configuration from JSON file
ParameterConfig config("config/shooter_game_config.json");

// Access parameter groups
const ParameterGroup* aiGroup = config.getParameterGroup("AI");
const ParameterDefinition* aggression = config.getParameter("AI", "aggressiveness");
```

#### Parameter Definition Structure
Each parameter includes:
- **Name and type** (float, int, bool, string)
- **Default, minimum, and maximum values**
- **Description** for documentation
- **Validation** and clamping functions

### 2. DDA Parameters with Configuration

#### New DDAParameters Class
```cpp
#include "DDAParameters.hpp"

// Create with configuration
ParameterConfig config("my_game_config.json");
DDAParameters params(&config);

// Access parameters generically
float aggression = params.getValue<float>("AI.aggressiveness", 0.5f);
params.setValue("AI.accuracy", 0.8f);

// Legacy compatibility still works
AIParameters legacyAI = params.getAIParameters();
params.setAIParameters(legacyAI);
```

### 3. Configurable Fitness Function

#### ConfigurableFitness Class
```cpp
#include "ConfigurableFitness.hpp"

// Load fitness configuration
ConfigurableFitness fitness;
fitness.loadFromFile("config/fitness_config.json");

// Evaluate fitness with configurable metrics
float score = fitness.evaluateFitness(parameters, metrics);

// Get detailed breakdown
std::string report = fitness.getEvaluationBreakdown(parameters, metrics);
```

#### Fitness Evaluation Types
- **Distance**: Penalize deviation from ideal value
- **Target**: Exponential penalty for missing target
- **Minimize**: Lower values are better
- **Maximize**: Higher values are better

### 4. Enhanced Genetic Algorithm

#### GAConfig Structure
```cpp
struct GAConfig {
    size_t populationSize = 50;
    size_t eliteSize = 5;
    float mutationRate = 0.1f;
    float crossoverRate = 0.7f;
    std::string selectionMethod = "tournament";
    std::string crossoverMethod = "uniform";
    std::string mutationMethod = "gaussian";
    // ... more options
};
```

#### Advanced GA Features
- **Multiple selection methods**: Tournament, roulette, rank-based
- **Various crossover strategies**: Uniform, single-point, multi-point
- **Adaptive mutation**: Gaussian, uniform, adaptive
- **Population diversity tracking**
- **Detailed evolution reports**

## JSON Configuration Files

### Parameter Configuration Example
```json
{
  "parameterGroups": [
    {
      "name": "AI",
      "description": "AI behavior parameters",
      "parameters": [
        {
          "name": "aggressiveness",
          "type": "float",
          "defaultValue": 0.6,
          "minValue": 0.0,
          "maxValue": 1.0,
          "description": "How aggressive the AI is in combat"
        }
      ]
    }
  ],
  "fitnessConfig": {
    "baseFitness": 100.0,
    "aggregationMethod": "weighted_sum",
    "metrics": [
      {
        "metricName": "player_deaths",
        "weight": 35.0,
        "idealValue": 0.15,
        "minValue": 0.0,
        "maxValue": 3.0,
        "evaluationType": "distance"
      }
    ]
  }
}
```

### Genetic Algorithm Configuration
```json
{
  "populationSize": 50,
  "eliteSize": 5,
  "mutationRate": 0.15,
  "crossoverRate": 0.8,
  "selectionMethod": "tournament",
  "crossoverMethod": "uniform",
  "mutationMethod": "gaussian"
}
```

## Usage Examples

### Basic Setup
```cpp
// 1. Load parameter configuration
ParameterConfig paramConfig("config/my_game.json");

// 2. Create DDA parameters
DDAParameters ddaParams(&paramConfig);
ddaParams.loadDefaults();

// 3. Setup configurable fitness
auto fitness = std::make_unique<ConfigurableFitness>(paramConfig.getFitnessConfig());

// 4. Configure genetic algorithm
GAConfig gaConfig;
gaConfig.loadFromFile("config/genetic_algorithm.json");
GeneticAlgorithm ga(gaConfig, &paramConfig);
ga.setFitnessEvaluator(std::move(fitness));

// 5. Initialize and evolve
ga.initializePopulation(ddaParams);
ga.evolve(metrics);
DDAParameters evolvedParams = ga.getBestIndividual();
```

### Custom Parameter Groups
You can define any parameter groups for your specific game:
```json
{
  "parameterGroups": [
    {
      "name": "Economy",
      "parameters": [
        {
          "name": "experienceMultiplier",
          "type": "float",
          "defaultValue": 1.0,
          "minValue": 0.5,
          "maxValue": 3.0
        }
      ]
    },
    {
      "name": "Environment", 
      "parameters": [
        {
          "name": "weatherIntensity",
          "type": "float",
          "defaultValue": 0.3,
          "minValue": 0.0,
          "maxValue": 1.0
        }
      ]
    }
  ]
}
```

### Parameter Interpolation
Smooth transitions between parameter sets:
```cpp
DDAParameters easyParams(&config);
DDAParameters hardParams(&config);

// Interpolate between difficulty levels
for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
    DDAParameters interpolated = DDAParameters::interpolate(easyParams, hardParams, t);
    // Use interpolated parameters...
}
```

## Game Type Presets

The system includes presets for common game types:
```cpp
// Fitness configurations for different genres
auto shooterFitness = FitnessUtils::createShooterGameFitness();
auto puzzleFitness = FitnessUtils::createPuzzleGameFitness();
auto platformerFitness = FitnessUtils::createPlatformerGameFitness();
auto rpgFitness = FitnessUtils::createRPGGameFitness();
```

## Migration from Hardcoded System

### Backward Compatibility
The new system maintains full backward compatibility:
```cpp
// Old code still works
AIParameters ai;
ai.aggressiveness = 0.7f;
DDAParameters oldStyle;
oldStyle.setAIParameters(ai);

// New configurable approach
DDAParameters newStyle(&config);
newStyle.setValue("AI.aggressiveness", 0.7f);
```

### Migration Steps
1. **Create JSON configuration** for your parameters
2. **Update parameter access** to use generic getValue/setValue
3. **Configure fitness function** through JSON instead of hardcoded logic
4. **Customize genetic algorithm** settings via configuration
5. **Test with existing metrics** to ensure compatibility

## Advanced Features

### Custom Fitness Functions
```cpp
class MyCustomFitness : public ConfigurableFitness {
public:
    float evaluateFitness(const ParameterValues& params, const MetricManager& metrics) const override {
        // Custom fitness evaluation logic
        float baseFitness = ConfigurableFitness::evaluateFitness(params, metrics);
        
        // Add custom calculations
        float customBonus = calculateCustomBonus(params);
        return baseFitness + customBonus;
    }
};
```

### Population Diversity Analysis
```cpp
GeneticAlgorithm ga(config, &paramConfig);
ga.initializePopulation(baseParams);

// Track diversity over generations
for (int gen = 0; gen < 10; ++gen) {
    ga.evolve(metrics);
    float diversity = ga.getPopulationDiversity();
    std::cout << "Generation " << gen << " diversity: " << diversity << std::endl;
}

// Get detailed evolution report
std::cout << ga.getEvolutionReport() << std::endl;
```

### Elite Individual Management
```cpp
// Inject specific high-performing individuals
ga.injectIndividual(knownGoodParams);

// Force specific elites
std::vector<DDAParameters> elites = {bestParams1, bestParams2};
ga.setEliteIndividuals(elites);
```

## Performance Considerations

- **JSON parsing** is done once during initialization
- **Parameter access** is cached for efficient runtime access  
- **Genetic operations** are optimized for large parameter spaces
- **Memory usage** scales linearly with parameter count

## Files Structure

```
DDAEngine_CPP/
├── includes/
│   ├── ParameterConfig.hpp         # Parameter definition system
│   ├── ConfigurableFitness.hpp     # Configurable fitness evaluation
│   ├── DDAParameters.hpp           # Enhanced parameter class
│   └── GeneticAlgorithm.hpp        # Enhanced genetic algorithm
├── src/
│   ├── ParameterConfig.cpp
│   ├── ConfigurableFitness.cpp
│   ├── DDAParameters.cpp
│   └── GeneticAlgorithm.cpp
├── config/
│   ├── default_parameters.json     # Default parameter configuration
│   ├── shooter_game_config.json    # Shooter game example
│   └── genetic_algorithm.json      # GA configuration
└── examples/
    └── configurable_dda_example.cpp # Complete usage example
```

## Building and Testing

```bash
# Build with CMake
mkdir build && cd build
cmake ..
make

# Run the configurable example
./ConfigurableDDAExample

# Test with your own configuration
./ConfigurableDDAExample path/to/your/config.json
```

## Conclusion

The configurable DDA system provides:
- **Maximum flexibility** through JSON configuration
- **Easy customization** for different game types
- **Powerful evolution strategies** with configurable genetic algorithms
- **Maintained compatibility** with existing code
- **Extensible architecture** for future enhancements

This system eliminates hardcoded limitations while providing a robust foundation for adaptive difficulty systems in any game genre.