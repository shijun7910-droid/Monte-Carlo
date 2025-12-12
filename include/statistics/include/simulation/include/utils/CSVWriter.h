#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

class CSVWriter {
public:
    static bool write(const std::vector<std::vector<double>>& data, 
                     const std::string& filename, 
                     const std::string& delimiter = ",") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                file << std::setprecision(10) << row[i];
                if (i < row.size() - 1) {
                    file << delimiter;
                }
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "Data written to " << filename << std::endl;
        return true;
    }
    
    static bool writePaths(const std::vector<std::vector<double>>& paths,
                          const std::string& filename,
                          const std::string& delimiter = ",") {
        if (paths.empty()) return false;
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        // 写入列标题
        file << "Path";
        for (size_t step = 0; step < paths[0].size(); ++step) {
            file << delimiter << "Step_" << step;
        }
        file << "\n";
        
        // 写入数据
        for (size_t i = 0; i < paths.size(); ++i) {
            file << i;
            for (double value : paths[i]) {
                file << delimiter << std::setprecision(10) << value;
            }
            file << "\n";
        }
        
        file.close();
        return true;
    }
    
    static bool writeSummary(const std::vector<double>& finalPrices,
                            const std::string& filename,
                            const std::vector<double>& percentiles = {0.01, 0.05, 0.25, 0.5, 0.75, 0.95, 0.99}) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        
        std::vector<double> sorted = finalPrices;
        std::sort(sorted.begin(), sorted.end());
        
        file << "Statistic,Value\n";
        file << "Count," << finalPrices.size() << "\n";
        file << "Mean," << Statistics::mean(finalPrices) << "\n";
        file << "Median," << Statistics::median(finalPrices) << "\n";
        file << "StdDev," << Statistics::standardDeviation(finalPrices) << "\n";
        file << "Min," << *std::min_element(finalPrices.begin(), finalPrices.end()) << "\n";
        file << "Max," << *std::max_element(finalPrices.begin(), finalPrices.end()) << "\n";
        
        for (double p : percentiles) {
            size_t index = static_cast<size_t>(p * sorted.size());
            if (index >= sorted.size()) index = sorted.size() - 1;
            file << "Percentile_" << (p * 100) << "," << sorted[index] << "\n";
        }
        
        file.close();
        return true;
    }
};

#endif // CSV_WRITER_H
