#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

class CSVWriter {
public:
    static bool write(const std::vector<std::vector<double>>& data, 
                     const std::string& filename, 
                     const std::vector<std::string>& headers = {}) {
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
        std::cout << "Data written to " << filename << std::endl;
        return true;
    }
    
    static bool writeVector(const std::vector<double>& data,
                           const std::string& filename,
                           const std::string& header = "Value") {
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
    
    static bool writePaths(const std::vector<std::vector<double>>& paths,
                          const std::string& filename,
                          bool includeIndex = true) {
        if (paths.empty()) return false;
        
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
    
    static bool writeSummary(const std::vector<double>& finalPrices,
                            const std::string& filename) {
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
        
        file.close();
        return true;
    }
    
    static std::string formatNumber(double value, int precision = 6) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
};

#endif // CSV_WRITER_H
