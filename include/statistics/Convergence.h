#ifndef CONVERGENCE_H
#define CONVERGENCE_H

#include <vector>
#include <cmath>
#include <algorithm>

class Convergence {
public:
    // Calculate standard error of the mean
    static double standardError(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;
        
        double stdDev = 0.0;
        double mean = 0.0;
        
        // Calculate mean
        for (double x : data) {
            mean += x;
        }
        mean /= data.size();
        
        // Calculate standard deviation
        for (double x : data) {
            stdDev += (x - mean) * (x - mean);
        }
        stdDev = std::sqrt(stdDev / (data.size() - 1));
        
        return stdDev / std::sqrt(data.size());
    }
    
    // Check convergence by comparing batch means
    static bool checkConvergence(const std::vector<double>& data,
                                int numBatches = 10,
                                double tolerance = 0.01) {
        if (data.size() < 2 * numBatches) return false;
        
        int batchSize = data.size() / numBatches;
        std::vector<double> batchMeans(numBatches);
        
        // Calculate batch means
        for (int i = 0; i < numBatches; ++i) {
            double sum = 0.0;
            for (int j = 0; j < batchSize; ++j) {
                sum += data[i * batchSize + j];
            }
            batchMeans[i] = sum / batchSize;
        }
        
        // Calculate overall mean
        double overallMean = 0.0;
        for (double mean : batchMeans) {
            overallMean += mean;
        }
        overallMean /= numBatches;
        
        // Calculate batch variance
        double batchVariance = 0.0;
        for (double mean : batchMeans) {
            batchVariance += (mean - overallMean) * (mean - overallMean);
        }
        batchVariance /= (numBatches - 1);
        
        // Standard error of batch means
        double standardError = std::sqrt(batchVariance / numBatches);
        
        // Relative error
        double relativeError = standardError / std::abs(overallMean);
        
        return relativeError < tolerance;
    }
    
    // Calculate convergence rate
    static std::vector<double> estimateConvergenceRate(
        const std::vector<double>& data,
        int minBatchSize = 100
    ) {
        std::vector<double> rates;
        
        if (data.size() < 2 * minBatchSize) return rates;
        
        int maxBatches = data.size() / minBatchSize;
        
        for (int numBatches = 2; numBatches <= maxBatches; ++numBatches) {
            int batchSize = data.size() / numBatches;
            
            std::vector<double> batchMeans(numBatches);
            for (int i = 0; i < numBatches; ++i) {
                double sum = 0.0;
                for (int j = 0; j < batchSize; ++j) {
                    sum += data[i * batchSize + j];
                }
                batchMeans[i] = sum / batchSize;
            }
            
            // Calculate batch variance
            double overallMean = 0.0;
            for (double mean : batchMeans) {
                overallMean += mean;
            }
            overallMean /= numBatches;
            
            double batchVariance = 0.0;
            for (double mean : batchMeans) {
                batchVariance += (mean - overallMean) * (mean - overallMean);
            }
            batchVariance /= (numBatches - 1);
            
            double standardError = std::sqrt(batchVariance / numBatches);
            rates.push_back(standardError);
        }
        
        return rates;
    }
    
    // Calculate effective sample size (ESS)
    static double effectiveSampleSize(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;
        
        // Calculate autocorrelation
        size_t n = data.size();
        double mean = 0.0;
        for (double x : data) {
            mean += x;
        }
        mean /= n;
        
        // Calculate variance
        double variance = 0.0;
        for (double x : data) {
            variance += (x - mean) * (x - mean);
        }
        variance /= (n - 1);
        
        // Calculate autocorrelation at lag 1
        double autocov = 0.0;
        for (size_t i = 1; i < n; ++i) {
            autocov += (data[i] - mean) * (data[i-1] - mean);
        }
        autocov /= (n - 2);
        
        double autocorr = autocov / variance;
        
        // ESS using initial positive sequence estimator
        double ess = n / (1 + 2 * std::abs(autocorr));
        
        return ess;
    }
    
    // Calculate Monte Carlo standard error
    static double monteCarloStandardError(const std::vector<double>& data) {
        if (data.size() < 2) return 0.0;
        
        double ess = effectiveSampleSize(data);
        if (ess < 1) return 0.0;
        
        double stdDev = 0.0;
        double mean = 0.0;
        
        for (double x : data) {
            mean += x;
        }
        mean /= data.size();
        
        for (double x : data) {
            stdDev += (x - mean) * (x - mean);
        }
        stdDev = std::sqrt(stdDev / (data.size() - 1));
        
        return stdDev / std::sqrt(ess);
    }
};

#endif // CONVERGENCE_H
