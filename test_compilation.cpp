#include <iostream>
#include "ParameterConfig.hpp"
#include "DDAParameters.hpp"
#include "GeneticAlgorithm.hpp"
#include "ConfigurableFitness.hpp"
#include "MetricManager.hpp"
#include "DDAEngine.hpp"
#include "DDAEngineAPI.h"

int main() {
    // Test basic functionality to ensure compilation works
    try {
        // Test parameter configuration
        ParameterConfig config = ParameterConfig::createDefaultConfig();
        
        // Test DDA parameters
        DDAParameters params(&config);
        params.loadDefaults();
        
        // Test that we can access values
        float aggression = params.getValue<float>("AI.aggressiveness", 0.5f);
        float diffMult = params.getValue<float>("Global.difficultyMultiplier", 1.0f);
        
        // Test legacy compatibility
        AIParameters aiParams = params.getAIParameters();
        
        std::cout << "Compilation test successful!" << std::endl;
        std::cout << "AI Aggressiveness: " << aggression << std::endl;
        std::cout << "Difficulty Multiplier: " << diffMult << std::endl;
        std::cout << "Legacy AI Aggressiveness: " << aiParams.aggressiveness << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}