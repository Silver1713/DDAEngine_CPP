#define TEST01 1 // Set to 1 to run the TEST01 example, 0 for the main demo
#ifndef TEST01
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>
#include "DDAEngine.hpp"
#include "DDAEngineAPI.h"

void simulateLevel(int levelNumber, bool isPlayerStruggling) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> timeDist(180.0, 420.0);
    std::uniform_real_distribution<> accuracyDist(0.4, 0.9);
    std::uniform_int_distribution<> deathsDist(0, 3);
    std::uniform_int_distribution<> enemiesDist(10, 50);
    std::uniform_real_distribution<> damageDist(50.0, 300.0);

    float completionTime = timeDist(gen);
    float accuracy = accuracyDist(gen);
    int deaths = deathsDist(gen);
    int enemiesKilled = enemiesDist(gen);
    float damageTaken = damageDist(gen);

    if (isPlayerStruggling) {
        completionTime *= 1.5f;
        accuracy *= 0.7f;
        deaths += 2;
        damageTaken *= 1.8f;
    }

    nlohmann::json metrics;
    metrics["level_id"] = "level_" + std::to_string(levelNumber);
    metrics["completion_time"] = completionTime;
    metrics["accuracy"] = accuracy;
    metrics["player_deaths"] = deaths;
    metrics["enemies_killed"] = enemiesKilled;
    metrics["damage_taken"] = damageTaken;
    metrics["powerups_collected"] = static_cast<int>(10 * accuracy);
    metrics["distance_traveled"] = 1000.0f + levelNumber * 100.0f;

    std::cout << "\n=== Level " << levelNumber << " Completed ===" << std::endl;
    std::cout << "Completion Time: " << completionTime << "s" << std::endl;
    std::cout << "Accuracy: " << (accuracy * 100) << "%" << std::endl;
    std::cout << "Deaths: " << deaths << std::endl;

    DDA_SubmitLevelMetrics(metrics.dump().c_str());
}

void printCurrentParameters() {
    AIParametersC ai;
    PCGParametersC pcg;
    DDA_GetAIParameters(&ai);
    DDA_GetPCGParameters(&pcg);

    std::cout << "\n=== Current DDA Parameters ===" << std::endl;
    std::cout << "AI Parameters:" << std::endl;
    std::cout << "  Aggressiveness: " << ai.aggressiveness << std::endl;
    std::cout << "  Reaction Time: " << ai.reactionTime << std::endl;
    std::cout << "  Accuracy: " << ai.accuracy << std::endl;
    std::cout << "  Movement Speed: " << ai.movementSpeed << std::endl;

    std::cout << "PCG Parameters:" << std::endl;
    std::cout << "  Enemy Density: " << pcg.enemyDensity << std::endl;
    std::cout << "  Power-up Frequency: " << pcg.powerUpFrequency << std::endl;
    std::cout << "  Obstacle Complexity: " << pcg.obstacleComplexity << std::endl;
    std::cout << "  Enemies per Room: " << pcg.minEnemiesPerRoom << "-" << pcg.maxEnemiesPerRoom << std::endl;

    std::cout << "Difficulty Multiplier: " << DDA_GetDifficultyMultiplier() << std::endl;
    std::cout << "Player Skill Level: " << DDA_GetPlayerSkillLevel() << std::endl;
}

int main() {
    std::cout << "=== DDA Engine Demo ===" << std::endl;

    DDA_Initialize();

    DDA_SetIdealMetrics(300.0f, 0.2f, 0.7f);

    DDA_SetMode(DDA_MODE_ADAPTIVE);
    DDA_SetEvolutionEnabled(1);

    std::cout << "\nInitial Parameters:" << std::endl;
    printCurrentParameters();

    bool playerStruggling = false;
    for (int session = 1; session <= 3; ++session) {
        std::cout << "\n\n========== SESSION " << session << " ==========" << std::endl;

        if (session == 2) {
            playerStruggling = true;
            std::cout << "** Player is now struggling **" << std::endl;
        }

        for (int level = 1; level <= 5; ++level) {
            simulateLevel((session - 1) * 5 + level, playerStruggling);

            DDA_UpdateAdaptive(0.1f);

            if (level % 2 == 0) {
                std::cout << "\nLevel Generation Hints:" << std::endl;
                std::cout << DDA_GetLevelGenerationHints() << std::endl;
            }
        }

        std::cout << "\n--- Evolution Phase ---" << std::endl;
        DDA_EvolveParameters();

        std::cout << "\nParameters after evolution:" << std::endl;
        printCurrentParameters();

        std::cout << "\nFull Parameter Export:" << std::endl;
        std::cout << DDA_ExportParametersJson() << std::endl;
    }

    std::cout << "\n\n=== Testing Different Modes ===" << std::endl;

    DDA_SetMode(DDA_MODE_FIXED);
    std::cout << "\nMode set to FIXED - parameters won't adapt" << std::endl;
    simulateLevel(100, false);
    printCurrentParameters();

    DDA_SetMode(DDA_MODE_LEARNING);
    std::cout << "\nMode set to LEARNING - continuous evolution" << std::endl;
    for (int i = 0; i < 3; ++i) {
        simulateLevel(200 + i, false);
        DDA_EvolveParameters();
    }

    std::cout << "\n\n=== Demo Complete ===" << std::endl;
    std::cout << "Final skill assessment: " << DDA_GetPlayerSkillLevel() << std::endl;

    return 0;
}
#endif

#ifdef TEST01
#include <iostream>
#include <random>
#include "DDAEngineAPI.h"

int main() {
    // 1) Load the DDA native plugin
    if (DDA_Load() != 0) {
        std::cerr << "DDA_Load failed" << std::endl;
        return 1;
    }

    // 2) Inline JSON configuration (updated)
    static std::string jsonConfig = R"({
  "parameterGroups": [
    {
      "name": "Enemies",
      "description": "Parameter Group for Enemies",
      "parameters": [
        { "name": "EnemyCount",         "type": "float", "defaultValue": 0.5, "minValue": 0, "maxValue": 10, "description": "Weight Enemy Count" },
        { "name": "SpawnRangeWeight",  "type": "float", "defaultValue": 1,   "minValue": 0, "maxValue": 10, "description": "Spawn Ranged Enemy Weight" },
        { "name": "SpawnContact",      "type": "float", "defaultValue": 1,   "minValue": 0, "maxValue": 10, "description": "Contact spawn" },
        { "name": "SpawnSwarm",        "type": "float", "defaultValue": 0,   "minValue": 0, "maxValue": 10, "description": "Spawn Swarm" },
        { "name": "TestMetric",        "type": "float", "defaultValue": 0,   "minValue": 0, "maxValue": 1,  "description": "Test" }
      ]
    }
  ],
  "fitnessConfig": {
    "baseFitness": 100,
    "aggregationMethod": "weighted_sum",
    "metrics": [
      {
        "metricName": "player.damageratio",
        "weight": 1.0,
        "idealValue": 0.5,
        "minValue": 0,
        "maxValue": 1,
        "evaluationType": "distance",
        "type": "count"
      },
      {
        "metricName": "Enemies.TestMetric",
        "weight": 0.233,
        "idealValue": 0.5,
        "minValue": 0,
        "maxValue": 1,
        "evaluationType": "distance",
		"type": "average"
      }
    ]
  }
})";

    // 3) Load configuration into DDA engine
    if (DDA_LoadConfig(&jsonConfig[0]) != 0) {
        std::cerr << "DDA_LoadConfig failed" << std::endl;
        return 1;
    }

    // 4) Initialize the DDA engine
    DDA_Initialize();

    // 5) Emulate multiple "sessions"
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> realDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> ratioDist(0.0f, 2.0f);

    for (int session = 0; session <50 ; ++session) {
        std::cout << "--- Session " << session << " ---" << std::endl;

        // a) Push some damageratio metrics
        for (int i = 0; i < 100; ++i) {
            float dmgRatio = ratioDist(rng);
            DDA_CollectMetric("player.damageratio", dmgRatio);
        }
        // b) Push TestMetric metrics
        for (int i = 0; i < 100; ++i) {
            float testVal = realDist(rng);
            DDA_CollectMetric("Enemies.TestMetric", testVal);
        }

        // 6) Evolve parameters based on collected metrics

        DDA_SetMode(DDA_MODE_LEARNING);
        

        
        DDA_AdvanceEngine();

        // 7) Export and print the resulting state JSON
        const char* stateJson = DDA_ExportParametersJson();
        std::cout << stateJson << std::endl;
        DDA_FreeString(stateJson);

        
    }
    // 8) Reset for next session
    DDA_Reset();
    // 9) Shutdown the DDA engine
    DDA_Shutdown();
    return 0;
}
#endif
