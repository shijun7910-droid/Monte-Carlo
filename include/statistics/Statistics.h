#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>
#include <iomanip>

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
    double quantile25 = 0.0;
    double quantile50 = 0.0;
    double quantile75 = 0.0;
    
    void print() const {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\nStatistical Summary:\n";
        std::cout << "=====================\n";
        std::cout << "Mean: " << mean << "\n";
        std::cout << "Median: " << median << "\n";
        std::cout << "Std Deviation: " << stdDev << "\n";
        std::cout << "Variance: " << variance << "\n";
        std::cout << "Minimum: " << min << "\n";
        std::cout << "Maximum: " << max << "\n";
        std::cout << "Skewness: " << skewness << "\n";
        std::cout << "Kurtosis: " << kurtosis << "\n";
        std::cout << "25th Percentile: " << quantile25 << "\n";
        std::cout << "50th Percentile: " << quantile50 << "\n";
        std::cout << "75th Percentile: " << quantile75 << "\n";
        std::cout << "95% CI: [" << confidenceInterval95.first 
                  << ", " << confidenceInterval95.second << "]\n";
        std::cout << "99% CI: [" << confidenceInterval99.first 
                  << ", " << confidenceInterval99.second << "]\n";
    }
};

class Statistics {
public:
    static double mean(const std::vector<double>& data) {
        if (data.empty()) return 0.0;
        return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
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
        return (sum / data.size()) - 3.0;
    }
    
    static double quantile(std::vector<double> data, double p) {
        if (data.empty()) return 0.0;
        if (p < 0.0) p = 0.0;
        if (p > 1.0) p = 1.0;
        
        std::sort(data.begin(), data.end());
        size_t n = data.size();
        double index = p * (n - 1);
        size_t i = static_cast<size_t>(index);
        double fraction = index - i;
        
        if (i + 1 < n) {
            return data[i] * (1 - fraction) + data[i + 1] * fraction;
        } else {
            return data[i];
        }
    }
    
    static std::pair<double, double> confidenceInterval(
        const std::vector<double>& data, 
        double confidence = 0.95
    ) {
        if (data.size() < 2) return {0.0, 0.0};
        
        double m = mean(data);
        double s = standardDeviation(data);
        double z = 1.96;  // 95% confidence
        
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
        summary.quantile25 = quantile(data, 0.25);
        summary.quantile50 = quantile(data, 0.50);
        summary.quantile75 = quantile(data, 0.75);
        summary.confidenceInterval95 = confidenceInterval(data, 0.95);
        summary.confidenceInterval99 = confidenceInterval(data, 0.99);
        
        return summary;
    }
};

#endif // STATISTICS_H
