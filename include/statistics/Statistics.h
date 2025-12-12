#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

struct StatisticalSummary {
    double mean = 0.0;
    double median = 0.0;
    double stdDev = 0.0;
    double variance = 0.0;
    double min = 0.0;
    double max = 0.0;
    double skewness = 0.0;
    double kurtosis = 0.0;
    std::pair<double, double> confidenceInterval95;
    std::pair<double, double> confidenceInterval99;
    double var95 = 0.0;
    double var99 = 0.0;
    double cvar95 = 0.0;
    double cvar99 = 0.0;
};

class Statistics {
public:
    static double mean(const std::vector<double>& data) {
        if (data.empty()) return 0.0;
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        return sum / data.size();
    }
    
    static double median(std::vector<double> data) {
        if (data.empty()) return 0.0;
        size_t n = data.size() / 2;
        std::nth_element(data.begin(), data.begin() + n, data.end());
        double median = data[n];
        
        if (data.size() % 2 == 0) {
            std::nth_element(data.begin(), data.begin() + n - 1, data.end());
            median = (median + data[n-1]) / 2.0;
        }
        
        return median;
    }
    
    static double standardDeviation(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;
        double m = mean(data);
        double sum = 0.0;
        for (double x : data) {
            sum += (x - m) * (x - m);
        }
        return std::sqrt(sum / (data.size() - 1));
    }
    
    static double variance(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;
        double m = mean(data);
        double sum = 0.0;
        for (double x : data) {
            sum += (x - m) * (x - m);
        }
        return sum / (data.size() - 1);
    }
    
    static double skewness(const std::vector<double>& data) {
        if (data.size() < 3) return 0.0;
        double m = mean(data);
        double s = standardDeviation(data);
        if (s == 0.0) return 0.0;
        
        double sum = 0.0;
        for (double x : data) {
            sum += std::pow((x - m) / s, 3);
        }
        return sum / data.size();
    }
    
    static double kurtosis(const std::vector<double>& data) {
        if (data.size() < 4) return 0.0;
        double m = mean(data);
        double s = standardDeviation(data);
        if (s == 0.0) return 0.0;
        
        double sum = 0.0;
        for (double x : data) {
            sum += std::pow((x - m) / s, 4);
        }
        return (sum / data.size()) - 3.0;  // 超额峰度
    }
    
    static double valueAtRisk(const std::vector<double>& data, double confidence) {
        if (data.empty()) return 0.0;
        std::vector<double> sortedData = data;
        std::sort(sortedData.begin(), sortedData.end());
        
        size_t index = static_cast<size_t>((1.0 - confidence) * sortedData.size());
        if (index >= sortedData.size()) index = sortedData.size() - 1;
        
        return sortedData[index];
    }
    
    static double conditionalVaR(const std::vector<double>& data, double confidence) {
        if (data.empty()) return 0.0;
        std::vector<double> sortedData = data;
        std::sort(sortedData.begin(), sortedData.end());
        
        size_t index = static_cast<size_t>((1.0 - confidence) * sortedData.size());
        if (index >= sortedData.size()) index = sortedData.size() - 1;
        
        double sum = 0.0;
        for (size_t i = 0; i <= index; ++i) {
            sum += sortedData[i];
        }
        
        return sum / (index + 1);
    }
    
    static std::pair<double, double> confidenceInterval(
        const std::vector<double>& data, 
        double confidence
    ) {
        if (data.size() < 2) return {0.0, 0.0};
        
        double m = mean(data);
        double s = standardDeviation(data);
        double z = 1.96;  // 95%置信度对应z值
        
        if (confidence == 0.99) {
            z = 2.576;
        } else if (confidence == 0.90) {
            z = 1.645;
        }
        
        double margin = z * s / std::sqrt(data.size());
        return {m - margin, m + margin};
    }
    
    static StatisticalSummary analyze(const std::vector<double>& data) {
        StatisticalSummary summary;
        
        if (data.empty()) return summary;
        
        summary.mean = mean(data);
        summary.median = median(data);
        summary.stdDev = standardDeviation(data);
        summary.variance = variance(data);
        summary.min = *std::min_element(data.begin(), data.end());
        summary.max = *std::max_element(data.begin(), data.end());
        summary.skewness = skewness(data);
        summary.kurtosis = kurtosis(data);
        summary.confidenceInterval95 = confidenceInterval(data, 0.95);
        summary.confidenceInterval99 = confidenceInterval(data, 0.99);
        summary.var95 = valueAtRisk(data, 0.95);
        summary.var99 = valueAtRisk(data, 0.99);
        summary.cvar95 = conditionalVaR(data, 0.95);
        summary.cvar99 = conditionalVaR(data, 0.99);
        
        return summary;
    }
    
    static void printSummary(const StatisticalSummary& summary, 
                           const std::string& title = "Statistical Summary") {
        std::cout << "\n" << title << "\n";
        std::cout << std::string(title.length(), '=') << "\n";
        std::cout << "Mean: " << summary.mean << "\n";
        std::cout << "Median: " << summary.median << "\n";
        std::cout << "Standard Deviation: " << summary.stdDev << "\n";
        std::cout << "Variance: " << summary.variance << "\n";
        std::cout << "Minimum: " << summary.min << "\n";
        std::cout << "Maximum: " << summary.max << "\n";
        std::cout << "Skewness: " << summary.skewness << "\n";
        std::cout << "Kurtosis: " << summary.kurtosis << "\n";
        std::cout << "95% Confidence Interval: [" 
                  << summary.confidenceInterval95.first << ", "
                  << summary.confidenceInterval95.second << "]\n";
        std::cout << "99% Confidence Interval: [" 
                  << summary.confidenceInterval99.first << ", "
                  << summary.confidenceInterval99.second << "]\n";
        std::cout << "Value at Risk (95%): " << summary.var95 << "\n";
        std::cout << "Value at Risk (99%): " << summary.var99 << "\n";
        std::cout << "Conditional VaR (95%): " << summary.cvar95 << "\n";
        std::cout << "Conditional VaR (99%): " << summary.cvar99 << "\n";
    }
};

#endif // STATISTICS_H
