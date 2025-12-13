#include "simulation/ResultAnalyzer.h"
#include <stdexcept>
#include <algorithm>

ResultAnalyzer::ResultAnalyzer(const std::vector<double>& prices) {
    setPrices(prices);
}

void ResultAnalyzer::setPrices(const std::vector<double>& prices) {
    if (prices.empty()) {
        throw std::invalid_argument("Prices vector cannot be empty");
    }
    
    prices_ = prices;
    calculateReturns();
}

StatisticalSummary ResultAnalyzer::analyzePrices() const {
    return Statistics::analyze(prices_);
}

StatisticalSummary ResultAnalyzer::analyzeReturns() const {
    return Statistics::analyze(returns_);
}

std::vector<double> ResultAnalyzer::calculateRiskMetrics(double confidence) const {
    if (confidence <= 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("Confidence level must be between 0 and 1");
    }
    
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
    
    // Sharpe Ratio (assuming 3% risk-free rate)
    double sharpe = RiskMetrics::calculateSharpeRatio(returns_, 0.03);
    metrics.push_back(sharpe);
    
    // Maximum Drawdown
    double drawdown = RiskMetrics::calculateMaxDrawdown(prices_);
    metrics.push_back(drawdown);
    
    return metrics;
}

std::vector<double> ResultAnalyzer::calculatePercentiles(
    const std::vector<double>& percentiles
) const {
    std::vector<double> results;
    
    for (double p : percentiles) {
        if (p < 0.0 || p > 1.0) {
            throw std::invalid_argument("Percentile must be between 0 and 1");
        }
        results.push_back(Statistics::quantile(prices_, p));
    }
    
    return results;
}

std::vector<double> ResultAnalyzer::calculateConvergenceMetrics() const {
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

void ResultAnalyzer::printAnalysis() const {
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

std::vector<std::vector<double>> ResultAnalyzer::generateReport() const {
    std::vector<std::vector<double>> report;
    
    // Add price statistics
    StatisticalSummary priceSummary = analyzePrices();
    report.push_back({
        priceSummary.mean,
        priceSummary.median,
        priceSummary.stdDev,
        priceSummary.min,
        priceSummary.max,
        priceSummary.skewness,
        priceSummary.kurtosis
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
    
    // Add convergence metrics
    std::vector<double> convMetrics = calculateConvergenceMetrics();
    report.push_back(convMetrics);
    
    return report;
}

// Calculate probability of reaching target price
double ResultAnalyzer::calculateProbability(double targetPrice) const {
    if (prices_.empty()) {
        return 0.0;
    }
    
    int count = 0;
    for (double price : prices_) {
        if (price >= targetPrice) {
            count++;
        }
    }
    
    return static_cast<double>(count) / prices_.size();
}

// Calculate expected shortfall for different confidence levels
std::vector<double> ResultAnalyzer::calculateExpectedShortfalls(
    const std::vector<double>& confidenceLevels
) const {
    std::vector<double> shortfalls;
    
    for (double confidence : confidenceLevels) {
        if (confidence <= 0.0 || confidence >= 1.0) {
            throw std::invalid_argument("Confidence level must be between 0 and 1");
        }
        shortfalls.push_back(RiskMetrics::calculateCVaR(returns_, confidence));
    }
    
    return shortfalls;
}

// Calculate distribution moments
std::vector<double> ResultAnalyzer::calculateMoments(int maxOrder) const {
    if (maxOrder < 1 || maxOrder > 4) {
        throw std::invalid_argument("Maximum order must be between 1 and 4");
    }
    
    std::vector<double> moments(maxOrder);
    
    double mean = Statistics::mean(prices_);
    moments[0] = mean;  // First moment (mean)
    
    if (maxOrder >= 2) {
        double variance = Statistics::variance(prices_);
        moments[1] = variance;  // Second moment (variance)
    }
    
    if (maxOrder >= 3) {
        moments[2] = Statistics::skewness(prices_);  // Third moment (skewness)
    }
    
    if (maxOrder >= 4) {
        moments[3] = Statistics::kurtosis(prices_);  // Fourth moment (kurtosis)
    }
    
    return moments;
}

void ResultAnalyzer::calculateReturns() {
    returns_.clear();
    if (prices_.size() < 2) return;
    
    double initialPrice = prices_[0];
    for (size_t i = 1; i < prices_.size(); ++i) {
        returns_.push_back((prices_[i] - initialPrice) / initialPrice);
    }
}
