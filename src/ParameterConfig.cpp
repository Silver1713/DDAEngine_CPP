#include "ParameterConfig.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

// ParameterDefinition implementation
nlohmann::json ParameterDefinition::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["type"] = type;
    j["description"] = description;
    
    // Handle variant serialization
    std::visit([&j](const auto& value) {
        j["defaultValue"] = value;
    }, defaultValue);
    
    std::visit([&j](const auto& value) {
        j["minValue"] = value;
    }, minValue);
    
    std::visit([&j](const auto& value) {
        j["maxValue"] = value;
    }, maxValue);
    
    return j;
}

void ParameterDefinition::fromJson(const nlohmann::json& j) {
    name = j.value("name", "");
    type = j.value("type", "float");
    description = j.value("description", "");
    
    // Deserialize based on type
    if (type == "float") {
        defaultValue = j.value("defaultValue", 0.0f);
        minValue = j.value("minValue", 0.0f);
        maxValue = j.value("maxValue", 1.0f);
    } else if (type == "int") {
        defaultValue = j.value("defaultValue", 0);
        minValue = j.value("minValue", 0);
        maxValue = j.value("maxValue", 100);
    } else if (type == "bool") {
        defaultValue = j.value("defaultValue", false);
        minValue = false;
        maxValue = true;
    } else if (type == "string") {
        defaultValue = j.value("defaultValue", std::string(""));
        minValue = std::string("");
        maxValue = std::string("");
    }
}

bool ParameterDefinition::isValid(const ParameterValue& value) const {
    if (type == "float") {
        try {
            float val = std::get<float>(value);
            float minVal = std::get<float>(minValue);
            float maxVal = std::get<float>(maxValue);
            return val >= minVal && val <= maxVal;
        } catch (const std::bad_variant_access&) {
            return false;
        }
    } else if (type == "int") {
        try {
            int val = std::get<int>(value);
            int minVal = std::get<int>(minValue);
            int maxVal = std::get<int>(maxValue);
            return val >= minVal && val <= maxVal;
        } catch (const std::bad_variant_access&) {
            return false;
        }
    } else if (type == "bool") {
        return std::holds_alternative<bool>(value);
    } else if (type == "string") {
        return std::holds_alternative<std::string>(value);
    }
    return false;
}

ParameterValue ParameterDefinition::clamp(const ParameterValue& value) const {
    if (type == "float") {
        try {
            float val = std::get<float>(value);
            float minVal = std::get<float>(minValue);
            float maxVal = std::get<float>(maxValue);
            return std::max(minVal, std::min(maxVal, val));
        } catch (const std::bad_variant_access&) {
            return defaultValue;
        }
    } else if (type == "int") {
        try {
            int val = std::get<int>(value);
            int minVal = std::get<int>(minValue);
            int maxVal = std::get<int>(maxValue);
            return std::max(minVal, std::min(maxVal, val));
        } catch (const std::bad_variant_access&) {
            return defaultValue;
        }
    }
    return value;
}

// ParameterGroup implementation
nlohmann::json ParameterGroup::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["description"] = description;
    j["parameters"] = nlohmann::json::array();
    
    for (const auto& param : parameters) {
        j["parameters"].push_back(param.toJson());
    }
    
    return j;
}

void ParameterGroup::fromJson(const nlohmann::json& j) {
    name = j.value("name", "");
    description = j.value("description", "");
    
    parameters.clear();
    if (j.contains("parameters") && j["parameters"].is_array()) {
        for (const auto& paramJson : j["parameters"]) {
            ParameterDefinition param;
            param.fromJson(paramJson);
            parameters.push_back(param);
        }
    }
}

const ParameterDefinition* ParameterGroup::getParameter(const std::string& name) const {
    auto it = std::find_if(parameters.begin(), parameters.end(),
        [&name](const ParameterDefinition& param) { return param.name == name; });
    return it != parameters.end() ? &(*it) : nullptr;
}

ParameterDefinition* ParameterGroup::getParameter(const std::string& name) {
    auto it = std::find_if(parameters.begin(), parameters.end(),
        [&name](const ParameterDefinition& param) { return param.name == name; });
    return it != parameters.end() ? &(*it) : nullptr;
}

// FitnessMetricConfig implementation
nlohmann::json FitnessMetricConfig::toJson() const {
    return nlohmann::json{
        {"metricName", metricName},
        {"weight", weight},
        {"idealValue", idealValue},
        {"minValue", minValue},
        {"maxValue", maxValue},
        {"evaluationType", evaluationType}
    };
}

void FitnessMetricConfig::fromJson(const nlohmann::json& j) {
    metricName = j.value("metricName", "");
    weight = j.value("weight", 1.0f);
    idealValue = j.value("idealValue", 0.0f);
    minValue = j.value("minValue", 0.0f);
    maxValue = j.value("maxValue", 1.0f);
    evaluationType = j.value("evaluationType", "distance");
}

// FitnessConfig implementation
nlohmann::json FitnessConfig::toJson() const {
    nlohmann::json j;
    j["baseFitness"] = baseFitness;
    j["aggregationMethod"] = aggregationMethod;
    j["metrics"] = nlohmann::json::array();
    
    for (const auto& metric : metrics) {
        j["metrics"].push_back(metric.toJson());
    }
    
    return j;
}

void FitnessConfig::fromJson(const nlohmann::json& j) {
    baseFitness = j.value("baseFitness", 100.0f);
    aggregationMethod = j.value("aggregationMethod", "weighted_sum");
    
    metrics.clear();
    if (j.contains("metrics") && j["metrics"].is_array()) {
        for (const auto& metricJson : j["metrics"]) {
            FitnessMetricConfig metric;
            metric.fromJson(metricJson);
            metrics.push_back(metric);
        }
    }
}

// ParameterConfig implementation
ParameterConfig::ParameterConfig(const std::string& configFilePath) : configPath(configFilePath) {
    loadFromFile(configFilePath);
}

bool ParameterConfig::loadFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        loadFromJson(j);
        configPath = filePath;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool ParameterConfig::saveToFile(const std::string& filePath) const {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to create config file: " << filePath << std::endl;
            return false;
        }
        
        nlohmann::json j = toJson();
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

void ParameterConfig::loadFromJson(const nlohmann::json& j) {
    parameterGroups.clear();
    
    if (j.contains("parameterGroups") && j["parameterGroups"].is_array()) {
        for (const auto& groupJson : j["parameterGroups"]) {
            ParameterGroup group;
            group.fromJson(groupJson);
            parameterGroups.push_back(group);
        }
    }
    
    if (j.contains("fitnessConfig")) {
        fitnessConfig.fromJson(j["fitnessConfig"]);
    }
}

nlohmann::json ParameterConfig::toJson() const {
    nlohmann::json j;
    
    j["parameterGroups"] = nlohmann::json::array();
    for (const auto& group : parameterGroups) {
        j["parameterGroups"].push_back(group.toJson());
    }
    
    j["fitnessConfig"] = fitnessConfig.toJson();
    
    return j;
}

void ParameterConfig::addParameterGroup(const ParameterGroup& group) {
    auto it = std::find_if(parameterGroups.begin(), parameterGroups.end(),
        [&group](const ParameterGroup& g) { return g.name == group.name; });
    
    if (it != parameterGroups.end()) {
        *it = group;
    } else {
        parameterGroups.push_back(group);
    }
}

const ParameterGroup* ParameterConfig::getParameterGroup(const std::string& name) const {
    auto it = std::find_if(parameterGroups.begin(), parameterGroups.end(),
        [&name](const ParameterGroup& group) { return group.name == name; });
    return it != parameterGroups.end() ? &(*it) : nullptr;
}

ParameterGroup* ParameterConfig::getParameterGroup(const std::string& name) {
    auto it = std::find_if(parameterGroups.begin(), parameterGroups.end(),
        [&name](const ParameterGroup& group) { return group.name == name; });
    return it != parameterGroups.end() ? &(*it) : nullptr;
}

const ParameterDefinition* ParameterConfig::getParameter(const std::string& groupName, const std::string& paramName) const {
    const ParameterGroup* group = getParameterGroup(groupName);
    return group ? group->getParameter(paramName) : nullptr;
}

bool ParameterConfig::validateParameterValues(const std::unordered_map<std::string, ParameterValue>& values) const {
    for (const auto& [key, value] : values) {
        // Parse key format: "groupName.paramName"
        size_t dotPos = key.find('.');
        if (dotPos == std::string::npos) continue;
        
        std::string groupName = key.substr(0, dotPos);
        std::string paramName = key.substr(dotPos + 1);
        
        const ParameterDefinition* param = getParameter(groupName, paramName);
        if (param && !param->isValid(value)) {
            return false;
        }
    }
    return true;
}

ParameterConfig ParameterConfig::createDefaultConfig() {
    ParameterConfig config;
    
    // AI Parameters Group
    ParameterGroup aiGroup;
    aiGroup.name = "AI";
    aiGroup.description = "AI behavior parameters";
    
    aiGroup.parameters = {
        {"aggressiveness", "float", 0.5f, 0.0f, 1.0f, "How aggressive the AI is"},
        {"reactionTime", "float", 1.0f, 0.1f, 3.0f, "AI reaction time in seconds"},
        {"accuracy", "float", 0.7f, 0.0f, 1.0f, "AI shooting accuracy"},
        {"movementSpeed", "float", 1.0f, 0.1f, 3.0f, "AI movement speed multiplier"},
        {"detectionRange", "float", 10.0f, 1.0f, 50.0f, "AI detection range"},
        {"attackFrequency", "float", 0.5f, 0.0f, 1.0f, "How often AI attacks"}
    };
    
    // PCG Parameters Group
    ParameterGroup pcgGroup;
    pcgGroup.name = "PCG";
    pcgGroup.description = "Procedural content generation parameters";
    
    pcgGroup.parameters = {
        {"enemyDensity", "float", 0.5f, 0.0f, 1.0f, "Density of enemies in levels"},
        {"powerUpFrequency", "float", 0.3f, 0.0f, 1.0f, "Frequency of power-ups"},
        {"obstacleComplexity", "float", 0.5f, 0.0f, 1.0f, "Complexity of obstacles"},
        {"pathBranching", "float", 0.4f, 0.0f, 1.0f, "Level path branching factor"},
        {"hazardIntensity", "float", 0.3f, 0.0f, 1.0f, "Intensity of environmental hazards"},
        {"minEnemiesPerRoom", "int", 1, 0, 10, "Minimum enemies per room"},
        {"maxEnemiesPerRoom", "int", 5, 1, 20, "Maximum enemies per room"}
    };
    
    config.addParameterGroup(aiGroup);
    config.addParameterGroup(pcgGroup);
    
    // Default fitness configuration
    FitnessConfig fitnessConfig;
    fitnessConfig.baseFitness = 100.0f;
    fitnessConfig.aggregationMethod = "weighted_sum";
    fitnessConfig.metrics = {
        {"player_deaths", 30.0f, 0.2f, 0.0f, 5.0f, "distance"},
        {"completion_time", 25.0f, 300.0f, 60.0f, 600.0f, "distance"},
        {"accuracy", 20.0f, 0.7f, 0.0f, 1.0f, "distance"}
    };
    
    config.setFitnessConfig(fitnessConfig);
    
    return config;
}

// ParameterValues implementation
void ParameterValues::setValue(const std::string& key, const ParameterValue& value) {
    if (config) {
        // Validate against configuration if available
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string groupName = key.substr(0, dotPos);
            std::string paramName = key.substr(dotPos + 1);
            const ParameterDefinition* param = config->getParameter(groupName, paramName);
            if (param) {
                values[key] = param->clamp(value);
                return;
            }
        }
    }
    values[key] = value;
}

bool ParameterValues::hasValue(const std::string& key) const {
    return values.find(key) != values.end();
}

void ParameterValues::setValues(const std::unordered_map<std::string, ParameterValue>& newValues) {
    for (const auto& [key, value] : newValues) {
        setValue(key, value);
    }
}

nlohmann::json ParameterValues::toJson() const {
    nlohmann::json j;
    for (const auto& [key, value] : values) {
        std::visit([&j, &key](const auto& val) {
            j[key] = val;
        }, value);
    }
    return j;
}

void ParameterValues::fromJson(const nlohmann::json& j) {
    values.clear();
    for (auto& [key, val] : j.items()) {
        if (val.is_number_float()) {
            setValue(key, val.get<float>());
        } else if (val.is_number_integer()) {
            setValue(key, val.get<int>());
        } else if (val.is_boolean()) {
            setValue(key, val.get<bool>());
        } else if (val.is_string()) {
            setValue(key, val.get<std::string>());
        }
    }
}

std::vector<float> ParameterValues::toGeneticVector() const {
    std::vector<float> genes;
    
    if (!config) {
        // Fallback: convert all numeric values
        for (const auto& [key, value] : values) {
            std::visit([&genes](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, float>) {
                    genes.push_back(val);
                } else if constexpr (std::is_same_v<T, int>) {
                    genes.push_back(static_cast<float>(val) / 10.0f); // Normalize ints
                } else if constexpr (std::is_same_v<T, bool>) {
                    genes.push_back(val ? 1.0f : 0.0f);
                }
            }, value);
        }
        return genes;
    }
    
    // Use configuration order
    for (const auto& group : config->getParameterGroups()) {
        for (const auto& param : group.parameters) {
            std::string key = group.name + "." + param.name;
            auto it = values.find(key);
            
            if (it != values.end()) {
                std::visit([&genes, &param](const auto& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, float>) {
                        // Normalize to 0-1 range based on min/max
                        float minVal = std::get<float>(param.minValue);
                        float maxVal = std::get<float>(param.maxValue);
                        float normalized = (val - minVal) / (maxVal - minVal);
                        genes.push_back(std::max(0.0f, std::min(1.0f, normalized)));
                    } else if constexpr (std::is_same_v<T, int>) {
                        int minVal = std::get<int>(param.minValue);
                        int maxVal = std::get<int>(param.maxValue);
                        float normalized = static_cast<float>(val - minVal) / static_cast<float>(maxVal - minVal);
                        genes.push_back(std::max(0.0f, std::min(1.0f, normalized)));
                    } else if constexpr (std::is_same_v<T, bool>) {
                        genes.push_back(val ? 1.0f : 0.0f);
                    }
                }, it->second);
            } else {
                // Use default value if not set
                std::visit([&genes, &param](const auto& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, float>) {
                        float minVal = std::get<float>(param.minValue);
                        float maxVal = std::get<float>(param.maxValue);
                        float normalized = (val - minVal) / (maxVal - minVal);
                        genes.push_back(std::max(0.0f, std::min(1.0f, normalized)));
                    } else if constexpr (std::is_same_v<T, int>) {
                        int minVal = std::get<int>(param.minValue);
                        int maxVal = std::get<int>(param.maxValue);
                        float normalized = static_cast<float>(val - minVal) / static_cast<float>(maxVal - minVal);
                        genes.push_back(std::max(0.0f, std::min(1.0f, normalized)));
                    } else if constexpr (std::is_same_v<T, bool>) {
                        genes.push_back(val ? 1.0f : 0.0f);
                    }
                }, param.defaultValue);
            }
        }
    }
    
    return genes;
}

void ParameterValues::fromGeneticVector(const std::vector<float>& genes) {
    if (!config) return;
    
    size_t geneIndex = 0;
    for (const auto& group : config->getParameterGroups()) {
        for (const auto& param : group.parameters) {
            if (geneIndex >= genes.size()) break;
            
            std::string key = group.name + "." + param.name;
            float normalizedValue = std::max(0.0f, std::min(1.0f, genes[geneIndex]));
            
            if (param.type == "float") {
                float minVal = std::get<float>(param.minValue);
                float maxVal = std::get<float>(param.maxValue);
                float actualValue = minVal + normalizedValue * (maxVal - minVal);
                setValue(key, actualValue);
            } else if (param.type == "int") {
                int minVal = std::get<int>(param.minValue);
                int maxVal = std::get<int>(param.maxValue);
                int actualValue = minVal + static_cast<int>(normalizedValue * (maxVal - minVal));
                setValue(key, actualValue);
            } else if (param.type == "bool") {
                setValue(key, normalizedValue > 0.5f);
            }
            
            geneIndex++;
        }
    }
}

void ParameterValues::clampValues() {
    if (!config) return;
    
    for (auto& [key, value] : values) {
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string groupName = key.substr(0, dotPos);
            std::string paramName = key.substr(dotPos + 1);
            const ParameterDefinition* param = config->getParameter(groupName, paramName);
            if (param) {
                value = param->clamp(value);
            }
        }
    }
}

bool ParameterValues::isValid() const {
    if (!config) return true;
    
    for (const auto& [key, value] : values) {
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string groupName = key.substr(0, dotPos);
            std::string paramName = key.substr(dotPos + 1);
            const ParameterDefinition* param = config->getParameter(groupName, paramName);
            if (param && !param->isValid(value)) {
                return false;
            }
        }
    }
    return true;
}