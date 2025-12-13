#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>

struct RiskMetrics {
    double mean;
    double standardDeviation;
    double var95;
    double cvar95;
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

#endif // STATISTICS_H
