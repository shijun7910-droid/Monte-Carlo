#include "Statistics.h"
#include <algorithm>
#include <numeric>
#include <cmath>

double Statistics::mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

double Statistics::standardDeviation(const std::vector<double>& data) {
    if (data.size() <= 1) return 0.0;
    double m = mean(data);
    double sum = 0.0;
    for (double x : data) {
        sum += (x - m) * (x - m);
    }
    return sqrt(sum / (data.size() - 1));
}

double Statistics::percentile(const std::vector<double>& data, double p) {
    if (data.empty()) return 0.0;
    if (p <= 0) return *std::min_element(data.begin(), data.end());
    if (p >= 1) return *std::max_element(data.begin(), data.end());
    
    std::vector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    
    double index = p * (sorted.size() - 1);
    int lower = static_cast<int>(index);
    int upper = lower + 1;
    
    if (upper >= sorted.size()) return sorted.back();
    
    double weight = index - lower;
    return sorted[lower] * (1 - weight) + sorted[upper] * weight;
}

double Statistics::valueAtRisk(const std::vector<double>& data, double confidence) {
    return percentile(data, 1 - confidence);
}

double Statistics::conditionalVaR(const std::vector<double>& data, double confidence) {
    double var = valueAtRisk(data, confidence);
    
    std::vector<double> losses;
    for (double x : data) {
        if (x <= var) {
            losses.push_back(x);
        }
    }
    
    if (losses.empty()) return var;
    
    return mean(losses);
}

std::vector<double> Statistics::computePercentiles(const std::vector<double>& data, 
                                                  const std::vector<double>& probs) {
    std::vector<double> result;
    for (double p : probs) {
        result.push_back(percentile(data, p));
    }
    return result;
}

RiskMetrics Statistics::calculateMetrics(const std::vector<double>& data) {
    RiskMetrics metrics;
    
    if (data.empty()) return metrics;
    
    std::vector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    
    metrics.mean = mean(data);
    metrics.standardDeviation = standardDeviation(data);
    metrics.minValue = sorted.front();
    metrics.maxValue = sorted.back();
    metrics.median = percentile(sorted, 0.5);
    metrics.var95 = valueAtRisk(sorted, 0.95);
    metrics.cvar95 = conditionalVaR(sorted, 0.95);
    
    // 计算常用百分位数
    std::vector<double> probs = {0.01, 0.05, 0.25, 0.75, 0.95, 0.99};
    metrics.percentiles = computePercentiles(sorted, probs);
    
    return metrics;
}
