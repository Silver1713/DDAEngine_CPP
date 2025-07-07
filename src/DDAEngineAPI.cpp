#include "DDAEngineAPI.h"
#include "DDAEngine.hpp"
#include <cstring>
#include <string>
#include <memory>

static std::unique_ptr<std::string> lastAllocatedString;
static std::unique_ptr<std::string> lastHintsString;

extern "C" {

DDA_API void DDA_Initialize() {
    DDAEngine::getInstance().initialize();
}

DDA_API void DDA_Shutdown() {
    
}

DDA_API void DDA_CollectMetric(const char* metricName, float value) {
    try {
        MetricManager::getInstance()->pushMetric(metricName, value);
    } catch (...) {
        
    }
}

DDA_API void DDA_CollectMetricInt(const char* metricName, int value) {
    try {
        MetricManager::getInstance()->pushMetric(metricName, value);
    } catch (...) {
        
    }
}

DDA_API void DDA_SubmitLevelMetrics(const char* jsonMetricMatrix) {
    if (jsonMetricMatrix) {
        DDAEngine::getInstance().collectLevelMetrics(jsonMetricMatrix);
    }
}

DDA_API void DDA_EvolveParameters() {
    DDAEngine::getInstance().evolveParameters();
}

DDA_API void DDA_GetAIParameters(AIParametersC* outParams) {
    if (outParams) {
        auto params = DDAEngine::getInstance().getCurrentParameters();
        outParams->aggressiveness = params.ai.aggressiveness;
        outParams->reactionTime = params.ai.reactionTime;
        outParams->accuracy = params.ai.accuracy;
        outParams->movementSpeed = params.ai.movementSpeed;
        outParams->detectionRange = params.ai.detectionRange;
        outParams->attackFrequency = params.ai.attackFrequency;
    }
}

DDA_API void DDA_GetPCGParameters(PCGParametersC* outParams) {
    if (outParams) {
        auto params = DDAEngine::getInstance().getCurrentParameters();
        outParams->enemyDensity = params.pcg.enemyDensity;
        outParams->powerUpFrequency = params.pcg.powerUpFrequency;
        outParams->obstacleComplexity = params.pcg.obstacleComplexity;
        outParams->pathBranching = params.pcg.pathBranching;
        outParams->hazardIntensity = params.pcg.hazardIntensity;
        outParams->minEnemiesPerRoom = params.pcg.minEnemiesPerRoom;
        outParams->maxEnemiesPerRoom = params.pcg.maxEnemiesPerRoom;
    }
}

DDA_API float DDA_GetDifficultyMultiplier() {
    return DDAEngine::getInstance().getCurrentParameters().difficultyMultiplier;
}

DDA_API void DDA_SetMode(DDAMode mode) {
    DDAEngine::getInstance().setMode(static_cast<::DDAMode>(mode));
}

DDA_API DDAMode DDA_GetMode() {
    return static_cast<DDAMode>(DDAEngine::getInstance().getMode());
}

DDA_API void DDA_SetEvolutionEnabled(int enabled) {
    DDAEngine::getInstance().setEvolutionEnabled(enabled != 0);
}

DDA_API int DDA_IsEvolutionEnabled() {
    return DDAEngine::getInstance().isEvolutionEnabledStatus() ? 1 : 0;
}

DDA_API void DDA_UpdateAdaptive(float deltaTime) {
    DDAEngine::getInstance().applyAdaptiveAdjustment(deltaTime);
}

DDA_API const char* DDA_GetLevelGenerationHints() {
    try {
        auto hints = DDAEngine::getInstance().getLevelGenerationHints();
        lastHintsString = std::make_unique<std::string>(hints.dump());
        return lastHintsString->c_str();
    } catch (...) {
        return "{}";
    }
}

DDA_API const char* DDA_ExportParametersJson() {
    try {
        lastAllocatedString = std::make_unique<std::string>(
            DDAEngine::getInstance().exportParametersAsJson()
        );
        return lastAllocatedString->c_str();
    } catch (...) {
        return "{}";
    }
}

DDA_API void DDA_ImportParametersJson(const char* json) {
    if (json) {
        DDAEngine::getInstance().importParametersFromJson(json);
    }
}

DDA_API float DDA_GetPlayerSkillLevel() {
    return DDAEngine::getInstance().calculatePlayerSkillLevel();
}

DDA_API void DDA_SetIdealMetrics(float completionTime, float deathRate, float accuracy) {
    DDAEngine::getInstance().setIdealMetrics(completionTime, deathRate, accuracy);
}

DDA_API void DDA_Reset() {
    DDAEngine::getInstance().resetToDefaults();
}

DDA_API void DDA_FreeString(const char* str) {
    
}

}