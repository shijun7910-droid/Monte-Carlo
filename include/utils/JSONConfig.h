#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

// Simple JSON parser for configuration files
class JSONConfig {
private:
    std::map<std::string, std::string> config_;
    std::string filename_;
    
    void parseLine(const std::string& line) {
        std::string trimmed = trim(line);
        
        // Skip empty lines and comments
        if (trimmed.empty() || trimmed[0] == '#') return;
        
        // Find key-value pair
        size_t colonPos = trimmed.find(':');
        if (colonPos == std::string::npos) return;
        
        std::string key = trim(trimmed.substr(0, colonPos));
        std::string value = trim(trimmed.substr(colonPos + 1));
        
        // Remove quotes if present
        if (!value.empty() && value[0] == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        
        config_[key] = value;
    }
    
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    
public:
    JSONConfig() = default;
    
    JSONConfig(const std::string& filename) {
        load(filename);
    }
    
    bool load(const std::string& filename) {
        filename_ = filename;
        config_.clear();
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open config file " << filename << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            parseLine(line);
        }
        
        file.close();
        return !config_.empty();
    }
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            return it->second;
        }
        return defaultValue;
    }
    
    double getDouble(const std::string& key, double defaultValue = 0.0) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            try {
                return std::stod(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            std::string value = it->second;
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return value == "true" || value == "1" || value == "yes";
        }
        return defaultValue;
    }
    
    std::vector<double> getDoubleArray(const std::string& key, 
                                      const std::vector<double>& defaultValue = {}) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            std::vector<double> result;
            std::stringstream ss(it->second);
            std::string item;
            
            while (std::getline(ss, item, ',')) {
                try {
                    result.push_back(std::stod(trim(item)));
                } catch (...) {
                    // Skip invalid entries
                }
            }
            
            return result;
        }
        return defaultValue;
    }
    
    bool hasKey(const std::string& key) const {
        return config_.find(key) != config_.end();
    }
    
    void setValue(const std::string& key, const std::string& value) {
        config_[key] = value;
    }
    
    void save(const std::string& filename = "") {
        std::string saveFile = filename.empty() ? filename_ : filename;
        
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
    }
    
    void print() const {
        std::cout << "\nConfiguration:\n";
        std::cout << "===============\n";
        for (const auto& [key, value] : config_) {
            std::cout << key << ": " << value << "\n";
        }
    }
};

#endif // JSON_CONFIG_H
