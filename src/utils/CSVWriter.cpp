#include "utils/CSVWriter.h"
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

bool CSVWriter::write(const std::vector<std::vector<double>>& data, 
                     const std::string& filename, 
                     const std::vector<std::string>& headers) {
    if (data.empty()) {
        throw std::invalid_argument("Data cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Write headers
    if (!headers.empty()) {
        for (size_t i = 0; i < headers.size(); ++i) {
            file << headers[i];
            if (i < headers.size() - 1) file << ",";
        }
        file << "\n";
    }
    
    // Write data
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << std::setprecision(10) << row[i];
            if (i < row.size() - 1) file << ",";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Data written to " << filename << " (" << data.size() << " rows)" << std::endl;
    return true;
}

bool CSVWriter::writeVector(const std::vector<double>& data,
                           const std::string& filename,
                           const std::string& header) {
    if (data.empty()) {
        throw std::invalid_argument("Data cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    file << header << "\n";
    for (double value : data) {
        file << std::setprecision(10) << value << "\n";
    }
    
    file.close();
    return true;
}

bool CSVWriter::writePaths(const std::vector<std::vector<double>>& paths,
                          const std::string& filename,
                          bool includeIndex) {
    if (paths.empty()) {
        throw std::invalid_argument("Paths cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Write headers
    if (includeIndex) {
        file << "Path";
        for (size_t step = 0; step < paths[0].size(); ++step) {
            file << ",Step_" << step;
        }
        file << "\n";
    }
    
    // Write data
    for (size_t i = 0; i < paths.size(); ++i) {
        if (includeIndex) {
            file << i;
        }
        for (size_t j = 0; j < paths[i].size(); ++j) {
            if (includeIndex || j > 0) file << ",";
            file << std::setprecision(10) << paths[i][j];
        }
        file << "\n";
    }
    
    file.close();
    return true;
}

bool CSVWriter::writeSummary(const std::vector<double>& finalPrices,
                            const std::string& filename) {
    if (finalPrices.empty()) {
        throw std::invalid_argument("Final prices cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Calculate statistics
    double mean = 0.0;
    double min = finalPrices[0];
    double max = finalPrices[0];
    for (double price : finalPrices) {
        mean += price;
        if (price < min) min = price;
        if (price > max) max = price;
    }
    mean /= finalPrices.size();
    
    double variance = 0.0;
    for (double price : finalPrices) {
        variance += (price - mean) * (price - mean);
    }
    variance /= (finalPrices.size() - 1);
    double stdDev = std::sqrt(variance);
    
    // Sort for percentiles
    std::vector<double> sorted = finalPrices;
    std::sort(sorted.begin(), sorted.end());
    
    // Write summary
    file << "Statistic,Value\n";
    file << "Count," << finalPrices.size() << "\n";
    file << "Mean," << mean << "\n";
    file << "Median," << sorted[sorted.size()/2] << "\n";
    file << "StdDev," << stdDev << "\n";
    file << "Min," << min << "\n";
    file << "Max," << max << "\n";
    file << "25th Percentile," << sorted[sorted.size()/4] << "\n";
    file << "75th Percentile," << sorted[3*sorted.size()/4] << "\n";
    file << "95th Percentile," << sorted[static_cast<size_t>(0.95 * sorted.size())] << "\n";
    file << "99th Percentile," << sorted[static_cast<size_t>(0.99 * sorted.size())] << "\n";
    
    file.close();
    return true;
}

std::string CSVWriter::formatNumber(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

// Write simulation parameters
bool CSVWriter::writeParameters(const std::map<std::string, std::string>& params,
                               const std::string& filename) {
    if (params.empty()) {
        throw std::invalid_argument("Parameters cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    file << "Parameter,Value\n";
    for (const auto& [key, value] : params) {
        file << key << "," << value << "\n";
    }
    
    file.close();
    return true;
}

// Write performance metrics
bool CSVWriter::writePerformanceMetrics(
    const std::map<std::string, double>& metrics,
    const std::string& filename) {
    
    if (metrics.empty()) {
        throw std::invalid_argument("Metrics cannot be empty");
    }
    
    // Create directory if it doesn't exist
    fs::path filePath(filename);
    fs::create_directories(filePath.parent_path());
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    file << "Metric,Value\n";
    for (const auto& [metric, value] : metrics) {
        file << metric << "," << std::setprecision(6) << value << "\n";
    }
    
    file.close();
    return true;
}
