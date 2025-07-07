#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

struct AIParameters {
    float aggressiveness = 0.5f;      
    float reactionTime = 1.0f;        
    float accuracy = 0.7f;            
    float movementSpeed = 1.0f;       
    float detectionRange = 10.0f;     
    float attackFrequency = 0.5f;     
    
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"aggressiveness", aggressiveness},
            {"reactionTime", reactionTime},
            {"accuracy", accuracy},
            {"movementSpeed", movementSpeed},
            {"detectionRange", detectionRange},
            {"attackFrequency", attackFrequency}
        };
    }
    
    void fromJson(const nlohmann::json& j) {
        aggressiveness = j.value("aggressiveness", aggressiveness);
        reactionTime = j.value("reactionTime", reactionTime);
        accuracy = j.value("accuracy", accuracy);
        movementSpeed = j.value("movementSpeed", movementSpeed);
        detectionRange = j.value("detectionRange", detectionRange);
        attackFrequency = j.value("attackFrequency", attackFrequency);
    }
};

struct PCGParameters {
    float enemyDensity = 0.5f;        
    float powerUpFrequency = 0.3f;    
    float obstacleComplexity = 0.5f;  
    float pathBranching = 0.4f;       
    float hazardIntensity = 0.3f;     
    int minEnemiesPerRoom = 1;        
    int maxEnemiesPerRoom = 5;        
    
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"enemyDensity", enemyDensity},
            {"powerUpFrequency", powerUpFrequency},
            {"obstacleComplexity", obstacleComplexity},
            {"pathBranching", pathBranching},
            {"hazardIntensity", hazardIntensity},
            {"minEnemiesPerRoom", minEnemiesPerRoom},
            {"maxEnemiesPerRoom", maxEnemiesPerRoom}
        };
    }
    
    void fromJson(const nlohmann::json& j) {
        enemyDensity = j.value("enemyDensity", enemyDensity);
        powerUpFrequency = j.value("powerUpFrequency", powerUpFrequency);
        obstacleComplexity = j.value("obstacleComplexity", obstacleComplexity);
        pathBranching = j.value("pathBranching", pathBranching);
        hazardIntensity = j.value("hazardIntensity", hazardIntensity);
        minEnemiesPerRoom = j.value("minEnemiesPerRoom", minEnemiesPerRoom);
        maxEnemiesPerRoom = j.value("maxEnemiesPerRoom", maxEnemiesPerRoom);
    }
};

struct DDAParameters {
    AIParameters ai;
    PCGParameters pcg;
    float difficultyMultiplier = 1.0f; 
    std::string profileName = "default";
    
    std::vector<float> toGeneticVector() const {
        return {
            ai.aggressiveness,
            ai.reactionTime,
            ai.accuracy,
            ai.movementSpeed,
            ai.detectionRange,
            ai.attackFrequency,
            pcg.enemyDensity,
            pcg.powerUpFrequency,
            pcg.obstacleComplexity,
            pcg.pathBranching,
            pcg.hazardIntensity,
            static_cast<float>(pcg.minEnemiesPerRoom) / 10.0f,
            static_cast<float>(pcg.maxEnemiesPerRoom) / 10.0f,
            difficultyMultiplier
        };
    }
    
    void fromGeneticVector(const std::vector<float>& genes) {
        if (genes.size() >= 14) {
            ai.aggressiveness = genes[0];
            ai.reactionTime = genes[1];
            ai.accuracy = genes[2];
            ai.movementSpeed = genes[3];
            ai.detectionRange = genes[4];
            ai.attackFrequency = genes[5];
            pcg.enemyDensity = genes[6];
            pcg.powerUpFrequency = genes[7];
            pcg.obstacleComplexity = genes[8];
            pcg.pathBranching = genes[9];
            pcg.hazardIntensity = genes[10];
            pcg.minEnemiesPerRoom = static_cast<int>(genes[11] * 10.0f);
            pcg.maxEnemiesPerRoom = static_cast<int>(genes[12] * 10.0f);
            difficultyMultiplier = genes[13];
        }
    }
    
    nlohmann::json toJson() const {
        return nlohmann::json{
            {"ai", ai.toJson()},
            {"pcg", pcg.toJson()},
            {"difficultyMultiplier", difficultyMultiplier},
            {"profileName", profileName}
        };
    }
    
    void fromJson(const nlohmann::json& j) {
        if (j.contains("ai")) ai.fromJson(j["ai"]);
        if (j.contains("pcg")) pcg.fromJson(j["pcg"]);
        difficultyMultiplier = j.value("difficultyMultiplier", difficultyMultiplier);
        profileName = j.value("profileName", profileName);
    }
    
    void clamp() {
        auto clampValue = [](float& value, float min, float max) {
            value = std::max(min, std::min(max, value));
        };
        
        clampValue(ai.aggressiveness, 0.0f, 1.0f);
        clampValue(ai.reactionTime, 0.1f, 3.0f);
        clampValue(ai.accuracy, 0.0f, 1.0f);
        clampValue(ai.movementSpeed, 0.1f, 3.0f);
        clampValue(ai.detectionRange, 1.0f, 50.0f);
        clampValue(ai.attackFrequency, 0.0f, 1.0f);
        
        clampValue(pcg.enemyDensity, 0.0f, 1.0f);
        clampValue(pcg.powerUpFrequency, 0.0f, 1.0f);
        clampValue(pcg.obstacleComplexity, 0.0f, 1.0f);
        clampValue(pcg.pathBranching, 0.0f, 1.0f);
        clampValue(pcg.hazardIntensity, 0.0f, 1.0f);
        pcg.minEnemiesPerRoom = std::max(0, std::min(10, pcg.minEnemiesPerRoom));
        pcg.maxEnemiesPerRoom = std::max(pcg.minEnemiesPerRoom, std::min(20, pcg.maxEnemiesPerRoom));
        
        clampValue(difficultyMultiplier, 0.1f, 3.0f);
    }
};