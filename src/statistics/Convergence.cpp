#include "statistics/Convergence.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

double Convergence::standardError(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    double mean = 0.0;
    for (double x : data) {
        mean += x;
    }
    mean /= data.size();
    
    double stdDev = 0.0;
    for (double x : data) {
        stdDev += (x - mean) * (x - mean);
    }
    stdDev = std::sqrt(stdDev / (data.size() - 1));
    
    return stdDev / std::sqrt(data.size());
}

bool Convergence::checkConvergence(const std::vector<double>& data,
                                 int numBatches,
                                 double tolerance) {
    if (data.size() < 2 * numBatches || numBatches < 2) {
        return false;
    }
    
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    
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
    double relativeError = std::abs(overallMean) > 1e-10 
                         ? standardError / std::abs(overallMean) 
                         : standardError;
    
    return relativeError < tolerance;
}

std::vector<double> Convergence::estimateConvergenceRate(
    const std::vector<double>& data,
    int minBatchSize
) {
    std::vector<double> rates;
    
    if (data.size() < 2 * minBatchSize || minBatchSize < 10) {
        return rates;
    }
    
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

double Convergence::effectiveSampleSize(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    // Calculate mean
    double mean = 0.0;
    for (double x : data) {
        mean += x;
    }
    mean /= data.size();
    
    // Calculate variance
    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= (data.size() - 1);
    
    if (variance == 0.0) {
        return data.size();
    }
    
    // Calculate autocorrelation at lags 1-10
    const int maxLag = std::min(10, static_cast<int>(data.size() / 2));
    double totalAutocorr = 0.0;
    int validLags = 0;
    
    for (int lag = 1; lag <= maxLag; ++lag) {
        double autocov = 0.0;
        int count = 0;
        
        for (size_t i = lag; i < data.size(); ++i) {
            autocov += (data[i] - mean) * (data[i - lag] - mean);
            count++;
        }
        
        if (count > 0) {
            double autocorr = autocov / (count * variance);
            if (autocorr > 0) {  // Only consider positive autocorrelation
                totalAutocorr += autocorr;
                validLags++;
            }
        }
    }
    
    double avgAutocorr = validLags > 0 ? totalAutocorr / validLags : 0.0;
    
    // ESS using initial positive sequence estimator
    double ess = data.size() / (1 + 2 * avgAutocorr);
    
    return std::min(ess, static_cast<double>(data.size()));
}

double Convergence::monteCarloStandardError(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    
    double ess = effectiveSampleSize(data);
    if (ess < 1) {
        return 0.0;
    }
    
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

// Calculate Gelman-Rubin statistic for multiple chains
double Convergence::gelmanRubinStatistic(
    const std::vector<std::vector<double>>& chains
) {
    if (chains.empty() || chains[0].empty()) {
        return 1.0;
    }
    
    size_t m = chains.size();  // Number of chains
    size_t n = chains[0].size();  // Samples per chain
    
    // Calculate within-chain means
    std::vector<double> chainMeans(m, 0.0);
    for (size_t i = 0; i < m; ++i) {
        for (double x : chains[i]) {
            chainMeans[i] += x;
        }
        chainMeans[i] /= n;
    }
    
    // Calculate overall mean
    double overallMean = 0.0;
    for (double mean : chainMeans) {
        overallMean += mean;
    }
    overallMean /= m;
    
    // Calculate between-chain variance
    double betweenVar = 0.0;
    for (double mean : chainMeans) {
        betweenVar += (mean - overallMean) * (mean - overallMean);
    }
    betweenVar = betweenVar / (m - 1) * n;
    
    // Calculate within-chain variances
    std::vector<double> chainVariances(m, 0.0);
    for (size_t i = 0; i < m; ++i) {
        for (double x : chains[i]) {
            chainVariances[i] += (x - chainMeans[i]) * (x - chainMeans[i]);
        }
        chainVariances[i] /= (n - 1);
    }
    
    // Calculate average within-chain variance
    double withinVar = 0.0;
    for (double var : chainVariances) {
        withinVar += var;
    }
    withinVar /= m;
    
    // Calculate pooled variance
    double pooledVar = (n - 1) / n * withinVar + 1.0 / n * betweenVar;
    
    // Gelman-Rubin statistic (potential scale reduction factor)
    return std::sqrt(pooledVar / withinVar);
}
