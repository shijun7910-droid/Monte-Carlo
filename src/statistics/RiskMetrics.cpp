#include "statistics/RiskMetrics.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

double RiskMetrics::calculateVaR(const std::vector<double>& returns, 
                               double confidence) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }
    if (confidence <= 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("Confidence level must be between 0 and 1");
    }
    
    std::vector<double> sorted = returns;
    std::sort(sorted.begin(), sorted.end());
    
    size_t index = static_cast<size_t>((1.0 - confidence) * sorted.size());
    if (index >= sorted.size()) {
        index = sorted.size() - 1;
    }
    
    return sorted[index];
}

double RiskMetrics::calculateCVaR(const std::vector<double>& returns,
                                double confidence) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }
    if (confidence <= 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("Confidence level must be between 0 and 1");
    }
    
    std::vector<double> sorted = returns;
    std::sort(sorted.begin(), sorted.end());
    
    size_t index = static_cast<size_t>((1.0 - confidence) * sorted.size());
    if (index >= sorted.size()) {
        index = sorted.size() - 1;
    }
    
    double sum = 0.0;
    for (size_t i = 0; i <= index; ++i) {
        sum += sorted[i];
    }
    
    return sum / (index + 1);
}

double RiskMetrics::calculateVolatility(const std::vector<double>& returns) {
    if (returns.size() < 2) {
        return 0.0;
    }
    
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) 
                / returns.size();
    double variance = 0.0;
    for (double r : returns) {
        variance += (r - mean) * (r - mean);
    }
    variance /= (returns.size() - 1);
    
    return std::sqrt(variance);
}

double RiskMetrics::calculateSharpeRatio(const std::vector<double>& returns,
                                       double riskFreeRate,
                                       int periodsPerYear) {
    if (returns.empty()) {
        return 0.0;
    }
    
    double meanReturn = std::accumulate(returns.begin(), returns.end(), 0.0)
                      / returns.size();
    double volatility = calculateVolatility(returns);
    
    if (volatility == 0.0) {
        return 0.0;
    }
    
    // Annualize returns and volatility
    double annualizedReturn = meanReturn * periodsPerYear;
    double annualizedVol = volatility * std::sqrt(periodsPerYear);
    
    return (annualizedReturn - riskFreeRate) / annualizedVol;
}

double RiskMetrics::calculateMaxDrawdown(const std::vector<double>& prices) {
    if (prices.empty()) {
        return 0.0;
    }
    
    double peak = prices[0];
    double maxDrawdown = 0.0;
    
    for (double price : prices) {
        if (price > peak) {
            peak = price;
        }
        double drawdown = (peak - price) / peak;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }
    }
    
    return maxDrawdown;
}

std::vector<double> RiskMetrics::calculatePortfolioRisk(
    const std::vector<std::vector<double>>& returns,
    const std::vector<double>& weights,
    double confidence
) {
    if (returns.empty() || returns[0].empty()) {
        throw std::invalid_argument("Returns data cannot be empty");
    }
    if (weights.empty() || weights.size() != returns.size()) {
        throw std::invalid_argument("Weights must match number of assets");
    }
    
    size_t nAssets = returns.size();
    size_t nReturns = returns[0].size();
    
    // Validate all return vectors have same size
    for (size_t i = 1; i < nAssets; ++i) {
        if (returns[i].size() != nReturns) {
            throw std::invalid_argument("All return vectors must have same size");
        }
    }
    
    std::vector<double> portfolioReturns;
    
    // Calculate portfolio returns
    for (size_t i = 0; i < nReturns; ++i) {
        double portReturn = 0.0;
        for (size_t j = 0; j < nAssets; ++j) {
            portReturn += weights[j] * returns[j][i];
        }
        portfolioReturns.push_back(portReturn);
    }
    
    double var = calculateVaR(portfolioReturns, confidence);
    double cvar = calculateCVaR(portfolioReturns, confidence);
    double vol = calculateVolatility(portfolioReturns);
    
    return {var, cvar, vol};
}

// Calculate portfolio variance using covariance matrix
double RiskMetrics::calculatePortfolioVariance(
    const std::vector<double>& weights,
    const std::vector<std::vector<double>>& covarianceMatrix
) {
    if (weights.empty() || covarianceMatrix.empty()) {
        return 0.0;
    }
    
    size_t n = weights.size();
    if (covarianceMatrix.size() != n) {
        throw std::invalid_argument("Covariance matrix dimensions must match weights");
    }
    
    double variance = 0.0;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            variance += weights[i] * weights[j] * covarianceMatrix[i][j];
        }
    }
    
    return variance;
}

// Calculate beta coefficient
double RiskMetrics::calculateBeta(
    const std::vector<double>& assetReturns,
    const std::vector<double>& marketReturns
) {
    if (assetReturns.size() != marketReturns.size() || assetReturns.size() < 2) {
        throw std::invalid_argument("Return vectors must have same size and at least 2 elements");
    }
    
    size_t n = assetReturns.size();
    
    // Calculate means
    double assetMean = 0.0;
    double marketMean = 0.0;
    for (size_t i = 0; i < n; ++i) {
        assetMean += assetReturns[i];
        marketMean += marketReturns[i];
    }
    assetMean /= n;
    marketMean /= n;
    
    // Calculate covariance and market variance
    double covariance = 0.0;
    double marketVariance = 0.0;
    
    for (size_t i = 0; i < n; ++i) {
        covariance += (assetReturns[i] - assetMean) * (marketReturns[i] - marketMean);
        marketVariance += (marketReturns[i] - marketMean) * (marketReturns[i] - marketMean);
    }
    
    covariance /= (n - 1);
    marketVariance /= (n - 1);
    
    if (marketVariance == 0.0) {
        return 0.0;
    }
    
    return covariance / marketVariance;
}
