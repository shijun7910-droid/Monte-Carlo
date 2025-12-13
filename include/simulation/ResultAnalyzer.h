#ifndef RESULT_ANALYZER_H
#define RESULT_ANALYZER_H

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"
#include "statistics/Convergence.h"

class ResultAnalyzer {
private:
    std::vector<double> prices_;
    std::vector<double> returns_;
    
public:
    ResultAnalyzer(const std::vector<double>& prices) {
        prices_ = prices;
        calculateReturns();
    }
    
    void setPrices(const std::vector<double>& prices) {
        prices_ = prices;
        calculateReturns();
    }
    
    StatisticalSummary analyzePrices() const {
        return Statistics::analyze(prices_);
    }
    
    StatisticalSummary analyzeReturns() const {
        return Statistics::analyze(returns_);
    }
    
    std::vector<double> calculateRiskMetrics(double confidence = 0.95) const {
        std::vector<double> metrics;
        
        // Value at Risk
        double var = RiskMetrics::calculateVaR(returns_, confidence);
        metrics.push_back(var);
        
        // Conditional VaR
        double cvar = RiskMetrics::calculateCVaR(returns_, confidence);
        metrics.push_back(cvar);
        
        // Volatility
        double volatility = RiskMetrics::calculateVolatility(returns_);
        metrics.push_back(volatility);
        
        // Sharpe Ratio
        double sharpe = RiskMetrics::calculateSharpeRatio(returns_);
        metrics.push_back(sharpe);
        
        // Maximum Drawdown
        double drawdown = RiskMetrics::calculateMaxDrawdown(prices_);
        metrics.push_back(drawdown);
        
        return metrics;
    }
    
    std::vector<double> calculatePercentiles(
        const std::vector<double>& percentiles = {0.01, 0.05, 0.25, 0.5, 0.75, 0.95, 0.99}
    ) const {
        return Statistics::calculatePercentiles(prices_, percentiles);
    }
    
    std::vector<double> calculateConvergenceMetrics() const {
        std::vector<double> metrics;
        
        // Standard error
        double se = Convergence::standardError(prices_);
        metrics.push_back(se);
        
        // Monte Carlo standard error
        double mcse = Convergence::monteCarloStandardError(prices_);
        metrics.push_back(mcse);
        
        // Effective sample size
        double ess = Convergence::effectiveSampleSize(prices_);
        metrics.push_back(ess);
        
        // Check convergence
        bool converged = Convergence::checkConvergence(prices_, 10, 0.01);
        metrics.push_back(converged ? 1.0 : 0.0);
        
        return metrics;
    }
    
    void printAnalysis() const {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n=== Result Analysis ===\n";
        
        // Price analysis
        std::cout << "\nPrice Analysis:\n";
        StatisticalSummary priceSummary = analyzePrices();
        priceSummary.print();
        
        // Return analysis
        std::cout << "\nReturn Analysis:\n";
        StatisticalSummary returnSummary = analyzeReturns();
        returnSummary.print();
        
        // Risk metrics
        std::cout << "\nRisk Metrics (95% confidence):\n";
        std::cout << "==============================\n";
        std::vector<double> riskMetrics = calculateRiskMetrics(0.95);
        std::vector<std::string> riskLabels = {
            "Value at Risk (VaR)",
            "Conditional VaR (CVaR)",
            "Volatility",
            "Sharpe Ratio",
            "Maximum Drawdown"
        };
        
        for (size_t i = 0; i < riskMetrics.size(); ++i) {
            std::cout << riskLabels[i] << ": " << riskMetrics[i] << "\n";
        }
        
        // Percentiles
        std::cout << "\nPrice Percentiles:\n";
        std::cout << "===================\n";
        std::vector<double> percentiles = calculatePercentiles();
        std::vector<double> percentileLevels = {1, 5, 25, 50, 75, 95, 99};
        
        for (size_t i = 0; i < percentiles.size(); ++i) {
            std::cout << percentileLevels[i] << "th percentile: " 
                      << percentiles[i] << "\n";
        }
        
        // Convergence metrics
        std::cout << "\nConvergence Metrics:\n";
        std::cout << "====================\n";
        std::vector<double> convMetrics = calculateConvergenceMetrics();
        std::vector<std::string> convLabels = {
            "Standard Error",
            "Monte Carlo SE",
            "Effective Sample Size",
            "Converged (1=yes, 0=no)"
        };
        
        for (size_t i = 0; i < convMetrics.size(); ++i) {
            std::cout << convLabels[i] << ": " << convMetrics[i] << "\n";
        }
    }
    
    std::vector<std::vector<double>> generateReport() const {
        std::vector<std::vector<double>> report;
        
        // Add price statistics
        StatisticalSummary priceSummary = analyzePrices();
        report.push_back({
            priceSummary.mean,
            priceSummary.median,
            priceSummary.stdDev,
            priceSummary.min,
            priceSummary.max
        });
        
        // Add return statistics
        StatisticalSummary returnSummary = analyzeReturns();
        report.push_back({
            returnSummary.mean,
            returnSummary.stdDev,
            returnSummary.skewness,
            returnSummary.kurtosis
        });
        
        // Add risk metrics
        std::vector<double> riskMetrics = calculateRiskMetrics(0.95);
        report.push_back(riskMetrics);
        
        // Add percentiles
        std::vector<double> percentiles = calculatePercentiles();
        report.push_back(percentiles);
        
        return report;
    }
    
private:
    void calculateReturns() {
        returns_.clear();
        if (prices_.size() < 2) return;
        
        double initialPrice = prices_[0];
        for (size_t i = 1; i < prices_.size(); ++i) {
            returns_.push_back((prices_[i] - initialPrice) / initialPrice);
        }
    }
};

#endif // RESULT_ANALYZER_H
