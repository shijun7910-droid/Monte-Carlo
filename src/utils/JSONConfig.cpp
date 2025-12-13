#include "utils/JSONConfig.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

// Helper function to trim whitespace
std::string JSONConfig::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// Helper function to convert to lowercase
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

JSONConfig::JSONConfig(const std::string& filename) {
    load(filename);
}

bool JSONConfig::load(const std::string& filename) {
    filename_ = filename;
    config_.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    bool inObject = false;
    
    while (std::getline(file, line)) {
        lineNumber++;
        std::string trimmed = trim(line);
        
        // Skip empty lines and comments
        if (trimmed.empty() || trimmed[0] == '#') continue;
        
        // Handle JSON structure
        if (trimmed[0] == '{') {
            inObject = true;
            continue;
        } else if (trimmed[0] == '}') {
            inObject = false;
            continue;
        }
        
        // Parse key-value pairs
        if (inObject) {
            parseLine(trimmed);
        }
    }
    
    file.close();
    return !config_.empty();
}

void JSONConfig::parseLine(const std::string& line) {
    // Remove trailing comma if present
    std::string trimmedLine = trim(line);
    if (!trimmedLine.empty() && trimmedLine.back() == ',') {
        trimmedLine.pop_back();
    }
    
    // Find key-value pair
    size_t colonPos = trimmedLine.find(':');
    if (colonPos == std::string::npos) return;
    
    std::string key = trim(trimmedLine.substr(0, colonPos));
    std::string value = trim(trimmedLine.substr(colonPos + 1));
    
    // Remove quotes if present
    if (!key.empty() && key.front() == '"' && key.back() == '"') {
        key = key.substr(1, key.length() - 2);
    }
    
    if (!value.empty() && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.length() - 2);
    }
    
    config_[key] = value;
}

std::string JSONConfig::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        return it->second;
    }
    return defaultValue;
}

double JSONConfig::getDouble(const std::string& key, double defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not convert '" << it->second 
                      << "' to double for key '" << key << "'. Using default: " 
                      << defaultValue << std::endl;
            return defaultValue;
        }
    }
    return defaultValue;
}

int JSONConfig::getInt(const std::string& key, int defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not convert '" << it->second 
                      << "' to int for key '" << key << "'. Using default: " 
                      << defaultValue << std::endl;
            return defaultValue;
        }
    }
    return defaultValue;
}

bool JSONConfig::getBool(const std::string& key, bool defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        std::string value = toLower(it->second);
        return value == "true" || value == "1" || value == "yes" || value == "on";
    }
    return defaultValue;
}

std::vector<double> JSONConfig::getDoubleArray(const std::string& key, 
                                              const std::vector<double>& defaultValue) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
        std::vector<double> result;
        std::stringstream ss(it->second);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            try {
                result.push_back(std::stod(trim(item)));
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not convert '" << item 
                          << "' to double in array for key '" << key << "'" << std::endl;
            }
        }
        
        return result;
    }
    return defaultValue;
}

bool JSONConfig::hasKey(const std::string& key) const {
    return config_.find(key) != config_.end();
}

void JSONConfig::setValue(const std::string& key, const std::string& value) {
    config_[key] = value;
}

void JSONConfig::save(const std::string& filename) {
    std::string saveFile = filename.empty() ? filename_ : filename;
    
    if (saveFile.empty()) {
        std::cerr << "Error: No filename specified for saving configuration" << std::endl;
        return;
    }
    
    std::ofstream file(saveFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing " << saveFile << std::endl;
        return;
    }
    
    file << "{\n";
    bool first = true;
    for (const auto& [key, value] : config_) {
        if (!first) file << ",\n";
        file << "  \"" << key << "\": \"" << value << "\"";
        first = false;
    }
    file << "\n}\n";
    
    file.close();
    std::cout << "Configuration saved to " << saveFile << std::endl;
}

void JSONConfig::print() const {
    std::cout << "\nConfiguration (" << config_.size() << " parameters):\n";
    std::cout << "===================================\n";
    for (const auto& [key, value] : config_) {
        std::cout << key << ": " << value << "\n";
    }
}

// Get nested configuration
std::shared_ptr<JSONConfig> JSONConfig::getNestedConfig(const std::string& prefix) const {
    auto nestedConfig = std::make_shared<JSONConfig>();
    
    for (const auto& [key, value] : config_) {
        if (key.find(prefix) == 0) {
            // Remove prefix and dot separator
            std::string nestedKey = key.substr(prefix.length());
            if (!nestedKey.empty() && nestedKey[0] == '.') {
                nestedKey = nestedKey.substr(1);
                nestedConfig->setValue(nestedKey, value);
            }
        }
    }
    
    return nestedConfig;
}

// Merge with another configuration
void JSONConfig::merge(const JSONConfig& other, bool overwrite) {
    for (const auto& [key, value] : other.config_) {
        if (overwrite || !hasKey(key)) {
            config_[key] = value;
        }
    }
}

// Clear all configuration
void JSONConfig::clear() {
    config_.clear();
}

// Get all keys
std::vector<std::string> JSONConfig::getKeys() const {
    std::vector<std::string> keys;
    for (const auto& [key, _] : config_) {
        keys.push_back(key);
    }
    return keys;
}

// Check if configuration is empty
bool JSONConfig::empty() const {
    return config_.empty();
}
