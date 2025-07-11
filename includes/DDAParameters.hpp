#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "ParameterConfig.hpp"

// Legacy structures for backward compatibility
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

// New configurable DDA parameters class
class DDAParameters {
private:
    ParameterValues parameters;
    const ParameterConfig* config;
    std::string profileName = "default";
    
public:
    // Constructors
    DDAParameters();
    explicit DDAParameters(const ParameterConfig* configuration);
    DDAParameters(const ParameterConfig* configuration, const std::string& profile);
    
    // Configuration management
    void setConfig(const ParameterConfig* configuration);
    const ParameterConfig* getConfig() const { return config; }
    
    // Parameter access
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const {
        return parameters.getValue<T>(key, defaultValue);
    }
    
    void setValue(const std::string& key, const ParameterValue& value);
    bool hasValue(const std::string& key) const;
    
    // Bulk operations
    void setParameters(const ParameterValues& params);
    const ParameterValues& getParameters() const { return parameters; }
    ParameterValues& getParameters() { return parameters; }
    
    // Legacy compatibility - AI parameters
    AIParameters getAIParameters() const;
    void setAIParameters(const AIParameters& ai);
    
    // Legacy compatibility - PCG parameters  
    PCGParameters getPCGParameters() const;
    void setPCGParameters(const PCGParameters& pcg);
    
    // Genetic algorithm support
    std::vector<float> toGeneticVector() const;
    void fromGeneticVector(const std::vector<float>& genes);
    
    // JSON serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Validation and clamping
    void clamp();
    bool isValid() const;
    
    // Profile management
    void setProfileName(const std::string& profile) { profileName = profile; }
    const std::string& getProfileName() const { return profileName; }
    
    // Utility functions
    void loadDefaults();
    void resetToDefaults();
    
    // Get all parameters by group
    std::unordered_map<std::string, ParameterValue> getParametersByGroup(const std::string& groupName) const;
    void setParametersByGroup(const std::string& groupName, const std::unordered_map<std::string, ParameterValue>& groupParams);
    
    // Parameter interpolation for smooth transitions
    static DDAParameters interpolate(const DDAParameters& from, const DDAParameters& to, float t);
    
    // Copy constructor and assignment for proper config handling
    DDAParameters(const DDAParameters& other);
    DDAParameters& operator=(const DDAParameters& other);
    
    // Comparison operators
    bool operator==(const DDAParameters& other) const;
    bool operator!=(const DDAParameters& other) const { return !(*this == other); }
};