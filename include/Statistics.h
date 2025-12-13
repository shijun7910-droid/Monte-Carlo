#pragma once
#include <vector>
#include <algorithm>
#include <cmath>

struct SimulationResults {
    std::vector<std::vector<double>> paths;
    std::vector<double> finalValues;
    std::vector<double> timePoints;
};

struct RiskMetrics {
    double mean;
    double standardDeviation;
    double var95;        // 95% VaR
    double cvar95;       // 95% CVaR
    double minValue;
    double maxValue;
    double median;
    std::vector<double> percentiles;
};

class Statistics {
public:
    static double mean(const std::vector<double>& data);
    static double standardDeviation(const std::vector<double>& data);
    static double percentile(const std::vector<double>& data, double p);
    static double valueAtRisk(const std::vector<double>& data, double confidence);
    static double conditionalVaR(const std::vector<double>& data, double confidence);
    static std::vector<double> computePercentiles(const std::vector<double>& data, 
                                                 const std::vector<double>& probs);
    
    static RiskMetrics calculateMetrics(const std::vector<double>& data);
};
