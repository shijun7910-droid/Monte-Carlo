#ifndef RISK_METRICS_H
#define RISK_METRICS_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

class RiskMetrics {
public:
    // Value at Risk (VaR) - Historical method
    static double calculateVaR(const std::vector<double>& returns, 
                             double confidence = 0.95) {
        if (returns.empty()) return 0.0;
        
        std::vector<double> sorted = returns;
        std::sort(sorted.begin(), sorted.end());
        
        size_t index = static_cast<size_t>((1.0 - confidence) * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        
        return sorted[index];
    }
    
    // Conditional VaR (CVaR) - Expected shortfall
    static double calculateCVaR(const std::vector<double>& returns,
                              double confidence = 0.95) {
        if (returns.empty()) return 0.0;
        
        std::vector<double> sorted = returns;
        std::sort(sorted.begin(), sorted.end());
        
        size_t index = static_cast<size_t>((1.0 - confidence) * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        
        double sum = 0.0;
        for (size_t i = 0; i <= index; ++i) {
            sum += sorted[i];
        }
        
        return sum / (index + 1);
    }
    
    // Calculate volatility (standard deviation)
    static double calculateVolatility(const std::vector<double>& returns) {
        if (returns.size() < 2) return 0.0;
        
        double mean = std::accumulate(returns.begin(), returns.end(), 0.0) 
                    / returns.size();
        double variance = 0.0;
        for (double r : returns) {
            variance += (r - mean) * (r - mean);
        }
        variance /= (returns.size() - 1);
        
        return std::sqrt(variance);
    }
    
    // Calculate Sharpe ratio
    static double calculateSharpeRatio(const std::vector<double>& returns,
                                     double riskFreeRate = 0.03,
                                     int periodsPerYear = 252) {
        if (returns.empty()) return 0.0;
        
        double meanReturn = std::accumulate(returns.begin(), returns.end(), 0.0)
                          / returns.size();
        double volatility = calculateVolatility(returns);
        
        if (volatility == 0.0) return 0.0;
        
        // Annualize
        double annualizedReturn = meanReturn * periodsPerYear;
        double annualizedVol = volatility * std::sqrt(periodsPerYear);
        
        return (annualizedReturn - riskFreeRate) / annualizedVol;
    }
    
    // Calculate maximum drawdown
    static double calculateMaxDrawdown(const std::vector<double>& prices) {
        if (prices.empty()) return 0.0;
        
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
    
    // Calculate expected shortfall for a portfolio
    static std::vector<double> calculatePortfolioRisk(
        const std::vector<std::vector<double>>& returns,
        const std::vector<double>& weights,
        double confidence = 0.95
    ) {
        size_t nAssets = returns.size();
        std::vector<double> portfolioReturns;
        
        if (nAssets == 0 || returns[0].empty()) {
            return {0.0, 0.0, 0.0};  // VaR, CVaR, volatility
        }
        
        size_t nReturns = returns[0].size();
        
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
};

#endif // RISK_METRICS_H
