#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <nlohmann/json.hpp>

// Supported parameter types
using ParameterValue = std::variant<float, int, bool, std::string>;

// Parameter definition with constraints
struct ParameterDefinition {
    std::string name;
    std::string type; // "float", "int", "bool", "string"
    ParameterValue defaultValue;
    ParameterValue minValue;
    ParameterValue maxValue;
    std::string description;
    
    // JSON serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Validation
    bool isValid(const ParameterValue& value) const;
    ParameterValue clamp(const ParameterValue& value) const;
};

// Parameter group (e.g., AI, PCG, Environment, etc.)
struct ParameterGroup {
    std::string name;
    std::string description;
    std::vector<ParameterDefinition> parameters;
    
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Get parameter by name
    const ParameterDefinition* getParameter(const std::string& name) const;
    ParameterDefinition* getParameter(const std::string& name);
};

// Fitness function configuration
struct FitnessMetricConfig {

    enum MetricType
    {
        SUM,
        AVERAGE,
        MINIMUM,
        MAXIMUM,
        COUNT,
        UNIQUE_COUNT,
        VARIANCE,
        STD_DEV,
    };


    std::string metricName;
    float weight = 1.0f;
    float idealValue = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    std::string evaluationType = "distance"; // "distance", "target", "minimize", "maximize"
    MetricType type = MetricType::AVERAGE;
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

struct FitnessConfig {
    std::vector<FitnessMetricConfig> metrics;
    float baseFitness = 100.0f;
    std::string aggregationMethod = "weighted_sum"; // "weighted_sum", "weighted_product"
    
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

// Main configuration class
class ParameterConfig {
private:
    std::vector<ParameterGroup> parameterGroups;
    FitnessConfig fitnessConfig;
    std::string configPath;
    
public:
    ParameterConfig() = default;
    explicit ParameterConfig(const std::string& configFilePath);
    
    // Load/Save configuration
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;
    void loadFromJson(const nlohmann::json& j);
    nlohmann::json toJson() const;
    
    // Parameter group management
    void addParameterGroup(const ParameterGroup& group);
    const ParameterGroup* getParameterGroup(const std::string& name) const;
    ParameterGroup* getParameterGroup(const std::string& name);
    const std::vector<ParameterGroup>& getParameterGroups() const { return parameterGroups; }
    
    // Parameter access
    const ParameterDefinition* getParameter(const std::string& groupName, const std::string& paramName) const;
    
    // Fitness configuration
    const FitnessConfig& getFitnessConfig() const { return fitnessConfig; }
	FitnessConfig& getFitnessConfig() { return fitnessConfig; }
    void setFitnessConfig(const FitnessConfig& config) { fitnessConfig = config; }
    
    // Validation
    bool validateParameterValues(const std::unordered_map<std::string, ParameterValue>& values) const;
    
    // Create default configuration
    static ParameterConfig createDefaultConfig();
};

// Parameter values container
class ParameterValues {
private:
    std::unordered_map<std::string, ParameterValue> values;
    const ParameterConfig* config;
    
public:
    explicit ParameterValues(const ParameterConfig* cfg = nullptr) : config(cfg) {}
    
    // Value access
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const;
    
    void setValue(const std::string& key, const ParameterValue& value);
    bool hasValue(const std::string& key) const;
    
    // Bulk operations
    void setValues(const std::unordered_map<std::string, ParameterValue>& newValues);
    const std::unordered_map<std::string, ParameterValue>& getValues() const { return values; }
    
    // JSON serialization
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    
    // Genetic algorithm support
    std::vector<float> toGeneticVector() const;
    void fromGeneticVector(const std::vector<float>& genes);
    
    // Validation and clamping
    void clampValues();
    bool isValid() const;
    
    // Configuration binding
    void setConfig(const ParameterConfig* cfg) { config = cfg; }
    const ParameterConfig* getConfig() const { return config; }
};

// Template implementation for getValue
template<typename T>
T ParameterValues::getValue(const std::string& key, const T& defaultValue) const {
    auto it = values.find(key);
    if (it == values.end()) {
        return defaultValue;
    }
    
    try {
        return std::get<T>(it->second);
    } catch (const std::bad_variant_access&) {
        return defaultValue;
    }
}