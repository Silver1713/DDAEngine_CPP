#include "DDAParameters.hpp"
#include <algorithm>
#include <iostream>

// Constructors
DDAParameters::DDAParameters() : config(nullptr), parameters(nullptr) {
    loadDefaults();
}

DDAParameters::DDAParameters(const ParameterConfig* configuration) 
    : config(configuration), parameters(configuration) {
    loadDefaults();
}

DDAParameters::DDAParameters(const ParameterConfig* configuration, const std::string& profile) 
    : config(configuration), parameters(configuration), profileName(profile) {
    loadDefaults();
}

// Copy constructor
DDAParameters::DDAParameters(const DDAParameters& other) 
    : config(other.config), parameters(other.parameters), profileName(other.profileName) {
    if (config) {
        parameters.setConfig(config);
    }
}

// Assignment operator
DDAParameters& DDAParameters::operator=(const DDAParameters& other) {
    if (this != &other) {
        config = other.config;
        parameters = other.parameters;
        profileName = other.profileName;
        if (config) {
            parameters.setConfig(config);
        }
    }
    return *this;
}

// Configuration management
void DDAParameters::setConfig(const ParameterConfig* configuration) {
    config = configuration;
    parameters.setConfig(configuration);
    loadDefaults();
}

// Parameter access
void DDAParameters::setValue(const std::string& key, const ParameterValue& value) {
    parameters.setValue(key, value);
}

bool DDAParameters::hasValue(const std::string& key) const {
    return parameters.hasValue(key);
}

// Bulk operations
void DDAParameters::setParameters(const ParameterValues& params) {
    parameters = params;
    if (config) {
        parameters.setConfig(config);
    }
}

// Legacy compatibility - AI parameters
AIParameters DDAParameters::getAIParameters() const {
    AIParameters ai;
    ai.aggressiveness = getValue<float>("AI.aggressiveness", 0.5f);
    ai.reactionTime = getValue<float>("AI.reactionTime", 1.0f);
    ai.accuracy = getValue<float>("AI.accuracy", 0.7f);
    ai.movementSpeed = getValue<float>("AI.movementSpeed", 1.0f);
    ai.detectionRange = getValue<float>("AI.detectionRange", 10.0f);
    ai.attackFrequency = getValue<float>("AI.attackFrequency", 0.5f);
    return ai;
}

void DDAParameters::setAIParameters(const AIParameters& ai) {
    setValue("AI.aggressiveness", ai.aggressiveness);
    setValue("AI.reactionTime", ai.reactionTime);
    setValue("AI.accuracy", ai.accuracy);
    setValue("AI.movementSpeed", ai.movementSpeed);
    setValue("AI.detectionRange", ai.detectionRange);
    setValue("AI.attackFrequency", ai.attackFrequency);
}

// Legacy compatibility - PCG parameters
PCGParameters DDAParameters::getPCGParameters() const {
    PCGParameters pcg;
    pcg.enemyDensity = getValue<float>("PCG.enemyDensity", 0.5f);
    pcg.powerUpFrequency = getValue<float>("PCG.powerUpFrequency", 0.3f);
    pcg.obstacleComplexity = getValue<float>("PCG.obstacleComplexity", 0.5f);
    pcg.pathBranching = getValue<float>("PCG.pathBranching", 0.4f);
    pcg.hazardIntensity = getValue<float>("PCG.hazardIntensity", 0.3f);
    pcg.minEnemiesPerRoom = getValue<int>("PCG.minEnemiesPerRoom", 1);
    pcg.maxEnemiesPerRoom = getValue<int>("PCG.maxEnemiesPerRoom", 5);
    return pcg;
}

void DDAParameters::setPCGParameters(const PCGParameters& pcg) {
    setValue("PCG.enemyDensity", pcg.enemyDensity);
    setValue("PCG.powerUpFrequency", pcg.powerUpFrequency);
    setValue("PCG.obstacleComplexity", pcg.obstacleComplexity);
    setValue("PCG.pathBranching", pcg.pathBranching);
    setValue("PCG.hazardIntensity", pcg.hazardIntensity);
    setValue("PCG.minEnemiesPerRoom", pcg.minEnemiesPerRoom);
    setValue("PCG.maxEnemiesPerRoom", pcg.maxEnemiesPerRoom);
}

// Genetic algorithm support
std::vector<float> DDAParameters::toGeneticVector() const {
    return parameters.toGeneticVector();
}

void DDAParameters::fromGeneticVector(const std::vector<float>& genes) {
    parameters.fromGeneticVector(genes);
}

// JSON serialization
nlohmann::json DDAParameters::toJson() const {
    nlohmann::json j;
    j["profileName"] = profileName;
    j["parameters"] = parameters.toJson();

#ifdef USE_LEGACY
    // Include legacy format for compatibility
    j["legacy"] = nlohmann::json{
        {"ai", getAIParameters().toJson()},
        {"pcg", getPCGParameters().toJson()}
    };
#endif
    
    return j;
}

void DDAParameters::fromJson(const nlohmann::json& j) {
    profileName = j.value("profileName", "default");
    
    if (j.contains("parameters")) {
        parameters.fromJson(j["parameters"]);
    }
#ifdef USE_LEGACY
	else if (j.contains("legacy")) {
        // Load from legacy format
        if (j["legacy"].contains("ai")) {
            AIParameters ai;
            ai.fromJson(j["legacy"]["ai"]);
            setAIParameters(ai);
        }
        if (j["legacy"].contains("pcg")) {
            PCGParameters pcg;
            pcg.fromJson(j["legacy"]["pcg"]);
            setPCGParameters(pcg);
        }
    }
#endif
}

// Validation and clamping
void DDAParameters::clamp() {
    parameters.clampValues();
}

bool DDAParameters::isValid() const {
    return parameters.isValid();
}

// Utility functions
void DDAParameters::loadDefaults() {
    if (!config) {
        // Load hardcoded defaults for backward compatibility
        setAIParameters(AIParameters{});
        setPCGParameters(PCGParameters{});
        return;
    }
    
    // Load defaults from configuration
    for (const auto& group : config->getParameterGroups()) {
        for (const auto& param : group.parameters) {
            std::string key = group.name + "." + param.name;
            if (!hasValue(key)) {
                std::visit([this, &key](const auto& defaultVal) {
                    setValue(key, defaultVal);
                }, param.defaultValue);
            }
        }
    }
}

void DDAParameters::resetToDefaults() {
    parameters = ParameterValues(config);
    loadDefaults();
}

// Get all parameters by group
std::unordered_map<std::string, ParameterValue> DDAParameters::getParametersByGroup(const std::string& groupName) const {
    std::unordered_map<std::string, ParameterValue> groupParams;
    
    if (!config) return groupParams;
    
    const ParameterGroup* group = config->getParameterGroup(groupName);
    if (!group) return groupParams;
    
    for (const auto& param : group->parameters) {
        std::string key = groupName + "." + param.name;
        if (hasValue(key)) {
            auto& allParams = parameters.getValues();
            auto it = allParams.find(key);
            if (it != allParams.end()) {
                groupParams[param.name] = it->second;
            }
        }
    }
    
    return groupParams;
}

void DDAParameters::setParametersByGroup(const std::string& groupName, const std::unordered_map<std::string, ParameterValue>& groupParams) {
    for (const auto& [paramName, value] : groupParams) {
        std::string key = groupName + "." + paramName;
        setValue(key, value);
    }
}

// Parameter interpolation for smooth transitions
DDAParameters DDAParameters::interpolate(const DDAParameters& from, const DDAParameters& to, float t) {
    if (from.config != to.config) {
        std::cerr << "Warning: Interpolating between parameters with different configurations" << std::endl;
    }
    
    DDAParameters result(from.config);
    t = std::max(0.0f, std::min(1.0f, t));
    
    if (!from.config) {
        // Fallback to legacy interpolation
        AIParameters fromAI = from.getAIParameters();
        AIParameters toAI = to.getAIParameters();
        PCGParameters fromPCG = from.getPCGParameters();
        PCGParameters toPCG = to.getPCGParameters();
        
        auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
        auto lerpInt = [](int a, int b, float t) { return static_cast<int>(a + (b - a) * t); };
        
        AIParameters resultAI;
        resultAI.aggressiveness = lerp(fromAI.aggressiveness, toAI.aggressiveness, t);
        resultAI.reactionTime = lerp(fromAI.reactionTime, toAI.reactionTime, t);
        resultAI.accuracy = lerp(fromAI.accuracy, toAI.accuracy, t);
        resultAI.movementSpeed = lerp(fromAI.movementSpeed, toAI.movementSpeed, t);
        resultAI.detectionRange = lerp(fromAI.detectionRange, toAI.detectionRange, t);
        resultAI.attackFrequency = lerp(fromAI.attackFrequency, toAI.attackFrequency, t);
        
        PCGParameters resultPCG;
        resultPCG.enemyDensity = lerp(fromPCG.enemyDensity, toPCG.enemyDensity, t);
        resultPCG.powerUpFrequency = lerp(fromPCG.powerUpFrequency, toPCG.powerUpFrequency, t);
        resultPCG.obstacleComplexity = lerp(fromPCG.obstacleComplexity, toPCG.obstacleComplexity, t);
        resultPCG.pathBranching = lerp(fromPCG.pathBranching, toPCG.pathBranching, t);
        resultPCG.hazardIntensity = lerp(fromPCG.hazardIntensity, toPCG.hazardIntensity, t);
        resultPCG.minEnemiesPerRoom = lerpInt(fromPCG.minEnemiesPerRoom, toPCG.minEnemiesPerRoom, t);
        resultPCG.maxEnemiesPerRoom = lerpInt(fromPCG.maxEnemiesPerRoom, toPCG.maxEnemiesPerRoom, t);
        
        result.setAIParameters(resultAI);
        result.setPCGParameters(resultPCG);
        return result;
    }
    
    // Configuration-based interpolation
    for (const auto& group : from.config->getParameterGroups()) {
        for (const auto& param : group.parameters) {
            std::string key = group.name + "." + param.name;
            
            if (param.type == "float") {
                float fromVal = from.getValue<float>(key, 0.0f);
                float toVal = to.getValue<float>(key, 0.0f);
                float resultVal = fromVal + (toVal - fromVal) * t;
                result.setValue(key, resultVal);
            } else if (param.type == "int") {
                int fromVal = from.getValue<int>(key, 0);
                int toVal = to.getValue<int>(key, 0);
                int resultVal = static_cast<int>(fromVal + (toVal - fromVal) * t);
                result.setValue(key, resultVal);
            } else if (param.type == "bool") {
                bool fromVal = from.getValue<bool>(key, false);
                bool toVal = to.getValue<bool>(key, false);
                bool resultVal = (t < 0.5f) ? fromVal : toVal;
                result.setValue(key, resultVal);
            } else if (param.type == "string") {
                std::string fromVal = from.getValue<std::string>(key, "");
                std::string toVal = to.getValue<std::string>(key, "");
                std::string resultVal = (t < 0.5f) ? fromVal : toVal;
                result.setValue(key, resultVal);
            }
        }
    }
    
    result.clamp();
    return result;
}

// Comparison operators
bool DDAParameters::operator==(const DDAParameters& other) const {
    if (config != other.config) return false;
    if (profileName != other.profileName) return false;
    
    const auto& thisValues = parameters.getValues();
    const auto& otherValues = other.parameters.getValues();
    
    if (thisValues.size() != otherValues.size()) return false;
    
    for (const auto& [key, value] : thisValues) {
        auto it = otherValues.find(key);
        if (it == otherValues.end()) return false;
        
        // Compare variant values
        if (value.index() != it->second.index()) return false;
        
        std::visit([&](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, float>) {
                auto otherVal = std::get<float>(it->second);
                if (std::abs(val - otherVal) > 1e-6f) return false;
            } else {
                if (val != std::get<T>(it->second)) return false;
            }
        }, value);
    }
    
    return true;
}