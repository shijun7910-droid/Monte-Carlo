#include "statistics/Statistics.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

// Helper function to check if data is valid
bool validateData(const std::vector<double>& data) {
    return !data.empty();
}

double Statistics::mean(const std::vector<double>& data) {
    if (!validateData(data)) {
        return 0.0;
    }
    
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

double Statistics::median(std::vector<double> data) {
    if (!validateData(data)) {
        return 0.0;
    }
    
    size_t n = data.size() / 2;
    std::nth_element(data.begin(), data.begin() + n, data.end());
    double median = data[n];
    
    if (data.size() % 2 == 0) {
        std::nth_element(data.begin(), data.begin() + n - 1, data.end());
        median = (median + data[n-1]) / 2.0;
    }
    
    return median;
}

double Statistics::standardDeviation(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    double m = mean(data);
    double sum = 0.0;
    for (double x : data) {
        sum += (x - m) * (x - m);
    }
    return std::sqrt(sum / (data.size() - 1));
}

double Statistics::variance(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    double m = mean(data);
    double sum = 0.0;
    for (double x : data) {
        sum += (x - m) * (x - m);
    }
    return sum / (data.size() - 1);
}

double Statistics::skewness(const std::vector<double>& data) {
    if (data.size() < 3) {
        return 0.0;
    }
    
    double m = mean(data);
    double s = standardDeviation(data);
    if (s == 0.0) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double x : data) {
        sum += std::pow((x - m) / s, 3);
    }
    return sum / data.size();
}

double Statistics::kurtosis(const std::vector<double>& data) {
    if (data.size() < 4) {
        return 0.0;
    }
    
    double m = mean(data);
    double s = standardDeviation(data);
    if (s == 0.0) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double x : data) {
        sum += std::pow((x - m) / s, 4);
    }
    return (sum / data.size()) - 3.0;  // Excess kurtosis
}

double Statistics::quantile(std::vector<double> data, double p) {
    if (!validateData(data)) {
        return 0.0;
    }
    
    if (p < 0.0) p = 0.0;
    if (p > 1.0) p = 1.0;
    
    std::sort(data.begin(), data.end());
    size_t n = data.size();
    
    if (p == 0.0) return data[0];
    if (p == 1.0) return data[n - 1];
    
    double index = p * (n - 1);
    size_t i = static_cast<size_t>(index);
    double fraction = index - i;
    
    if (i + 1 < n) {
        return data[i] * (1 - fraction) + data[i + 1] * fraction;
    } else {
        return data[i];
    }
}

std::pair<double, double> Statistics::confidenceInterval(
    const std::vector<double>& data, 
    double confidence
) {
    if (data.size() < 2 || confidence <= 0.0 || confidence >= 1.0) {
        return {0.0, 0.0};
    }
    
    double m = mean(data);
    double s = standardDeviation(data);
    
    // Z-scores for common confidence levels
    double z = 1.96;  // 95% confidence
    
    if (confidence == 0.99) {
        z = 2.576;
    } else if (confidence == 0.90) {
        z = 1.645;
    } else if (confidence == 0.80) {
        z = 1.282;
    }
    
    double margin = z * s / std::sqrt(data.size());
    return {m - margin, m + margin};
}

StatisticalSummary Statistics::analyze(const std::vector<double>& data) {
    StatisticalSummary summary;
    
    if (!validateData(data)) {
        return summary;
    }
    
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

void StatisticalSummary::print() const {
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
