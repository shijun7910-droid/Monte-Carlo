#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "models/GBM.h"
#include "simulation/MonteCarloSimulator.h"
#include "simulation/PathGenerator.h"
#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"
#include "utils/CSVWriter.h"
#include "utils/Timer.h"
#include "utils/Logger.h"

// Structure for currency pair
struct CurrencyPair {
    std::string name;
    std::shared_ptr<GBM> model;
    double weight;  // For portfolio construction
};

// Structure for portfolio
struct Portfolio {
    std::vector<CurrencyPair> currencies;
    std::vector<double> weights;
    
    double calculateValue(const std::vector<double>& prices) const {
        double total = 0.0;
        for (size_t i = 0; i < currencies.size(); ++i) {
            total += currencies[i].weight * prices[i];
        }
        return total;
    }
};

int main() {
    // Setup logger
    Logger::getInstance().setConsoleOutput(true);
    Logger::getInstance().setMinLevel(LogLevel::INFO);
    
    std::cout << "=== Multi-Currency Portfolio Simulation ===\n\n";
    
    Timer totalTimer("Total Simulation Time");
    
    // Define currency pairs
    std::vector<CurrencyPair> currencies = {
        {"USD/RUB", std::make_shared<GBM>(75.0, 0.05, 0.25), 0.4},
        {"EUR/USD", std::make_shared<GBM>(1.10, 0.02, 0.15), 0.3},
        {"GBP/USD", std::make_shared<GBM>(1.30, 0.03, 0.18), 0.2},
        {"USD/JPY", std::make_shared<GBM>(110.0, 0.01, 0.12), 0.1}
    };
    
    // Correlation matrix (simplified)
    std::vector<std::vector<double>> correlationMatrix = {
        {1.00, 0.30, 0.25, 0.10},  // USD/RUB
        {0.30, 1.00, 0.70, 0.40},  // EUR/USD
        {0.25, 0.70, 1.00, 0.35},  // GBP/USD
        {0.10, 0.40, 0.35, 1.00}   // USD/JPY
    };
    
    // Simulation parameters
    const int numSimulations = 10000;
    const int numSteps = 252;
    const double dt = 1.0 / 252.0;
    
    std::cout << "Simulating " << numSimulations << " paths for " 
              << currencies.size() << " currency pairs...\n";
    std::cout << "Time horizon: " << numSteps << " steps (" 
              << numSteps * dt << " years)\n\n";
    
    // Store results for each currency
    std::vector<std::vector<double>> finalPrices(currencies.size());
    std::vector<std::vector<double>> allReturns(currencies.size());
    std::vector<double> portfolioValues(numSimulations, 0.0);
    std::vector<double> portfolioReturns(numSimulations, 0.0);
    
    double initialPortfolioValue = 0.0;
    for (const auto& currency : currencies) {
        initialPortfolioValue += currency.weight * currency.model->getInitialPrice();
    }
    
    // Run simulations for each currency
    for (size_t i = 0; i < currencies.size(); ++i) {
        std::cout << "Simulating " << currencies[i].name << "...\n";
        
        Timer timer(currencies[i].name + " simulation");
        
        auto simulator = std::make_shared<MonteCarloSimulator>(currencies[i].model);
        simulator->setSeed(42 + i);
        
        auto result = simulator->runSimulation(numSimulations, numSteps, dt);
        finalPrices[i] = result.finalPrices;
        allReturns[i] = result.returns;
        
        // Accumulate portfolio values
        for (int j = 0; j < numSimulations; ++j) {
            portfolioValues[j] += currencies[i].weight * result.finalPrices[j];
        }
    }
    
    // Calculate portfolio returns
    for (int i = 0; i < numSimulations; ++i) {
        portfolioReturns[i] = (portfolioValues[i] - initialPortfolioValue) / initialPortfolioValue;
    }
    
    // Analyze results
    std::cout << "\n=== Portfolio Analysis ===\n\n";
    
    // Individual currency analysis
    std::cout << "Individual Currency Results:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::setw(15) << "Currency" 
              << std::setw(15) << "Initial" 
              << std::setw(15) << "Mean" 
              << std::setw(15) << "Std Dev" 
              << std::setw(20) << "95% CI\n";
    
    for (size_t i = 0; i < currencies.size(); ++i) {
        auto summary = Statistics::analyze(finalPrices[i]);
        
        std::cout << std::setw(15) << currencies[i].name
                  << std::setw(15) << std::fixed << std::setprecision(2) 
                  << currencies[i].model->getInitialPrice()
                  << std::setw(15) << summary.mean
                  << std::setw(15) << summary.stdDev
                  << std::setw(10) << "[" << summary.confidenceInterval95.first
                  << ", " << summary.confidenceInterval95.second << "]\n";
    }
    
    // Portfolio analysis
    std::cout << "\nPortfolio Results:\n";
    std::cout << std::string(80, '-') << "\n";
    
    auto portfolioPriceSummary = Statistics::analyze(portfolioValues);
    auto portfolioReturnSummary = Statistics::analyze(portfolioReturns);
    
    std::cout << "Initial Portfolio Value: " << initialPortfolioValue << "\n";
    std::cout << "Mean Portfolio Value: " << portfolioPriceSummary.mean << "\n";
    std::cout << "Portfolio Return: " << portfolioReturnSummary.mean * 100 << "%\n";
    std::cout << "Portfolio Volatility: " << portfolioReturnSummary.stdDev * std::sqrt(252) * 100 << "%\n";
    std::cout << "Sharpe Ratio (3% risk-free): " 
              << RiskMetrics::calculateSharpeRatio(portfolioReturns, 0.03) << "\n";
    
    // Risk metrics
    double var95 = RiskMetrics::calculateVaR(portfolioReturns, 0.95);
    double cvar95 = RiskMetrics::calculateCVaR(portfolioReturns, 0.95);
    double maxDrawdown = RiskMetrics::calculateMaxDrawdown(portfolioValues);
    
    std::cout << "\nRisk Metrics:\n";
    std::cout << "95% Value at Risk: " << var95 * 100 << "%\n";
    std::cout << "95% Conditional VaR: " << cvar95 * 100 << "%\n";
    std::cout << "Maximum Drawdown: " << maxDrawdown * 100 << "%\n";
    
    // Correlation analysis
    std::cout << "\nCorrelation Matrix:\n";
    std::cout << std::string(60, '-') << "\n";
    
    std::cout << std::setw(15) << " ";
    for (const auto& currency : currencies) {
        std::cout << std::setw(15) << currency.name;
    }
    std::cout << "\n";
    
    for (size_t i = 0; i < currencies.size(); ++i) {
        std::cout << std::setw(15) << currencies[i].name;
        for (size_t j = 0; j < currencies.size(); ++j) {
            if (i == j) {
                std::cout << std::setw(15) << "1.00";
            } else {
                // Calculate empirical correlation
                double corr = 0.0;
                double mean_i = Statistics::mean(allReturns[i]);
                double mean_j = Statistics::mean(allReturns[j]);
                double std_i = Statistics::standardDeviation(allReturns[i]);
                double std_j = Statistics::standardDeviation(allReturns[j]);
                
                if (std_i > 0 && std_j > 0) {
                    for (int k = 0; k < numSimulations; ++k) {
                        corr += (allReturns[i][k] - mean_i) * (allReturns[j][k] - mean_j);
                    }
                    corr /= (numSimulations - 1) * std_i * std_j;
                }
                
                std::cout << std::setw(15) << std::fixed << std::setprecision(3) << corr;
            }
        }
        std::cout << "\n";
    }
    
    // Save results
    std::cout << "\nSaving results...\n";
    
    // Save portfolio values
    CSVWriter::writeVector(portfolioValues, "portfolio_values.csv", "PortfolioValue");
    
    // Save individual currency results
    for (size_t i = 0; i < currencies.size(); ++i) {
        std::string filename = currencies[i].name + "_prices.csv";
        std::replace(filename.begin(), filename.end(), '/', '_');
        CSVWriter::writeVector(finalPrices[i], filename, "Price");
    }
    
    // Save summary statistics
    std::vector<std::vector<std::string>> summaryData;
    for (size_t i = 0; i < currencies.size(); ++i) {
        auto summary = Statistics::analyze(finalPrices[i]);
        summaryData.push_back({
            currencies[i].name,
            std::to_string(currencies[i].model->getInitialPrice()),
            std::to_string(summary.mean),
            std::to_string(summary.stdDev),
            std::to_string(summary.confidenceInterval95.first),
            std::to_string(summary.confidenceInterval95.second)
        });
    }
    
    std::vector<std::string> headers = {"Currency", "Initial", "Mean", "StdDev", "CI_Lower", "CI_Upper"};
    CSVWriter::write(summaryData, "currency_summary.csv", headers);
    
    std::cout << "\nResults saved to CSV files.\n";
    std::cout << "Use Python scripts for visualization and further analysis.\n";
    
    totalTimer.stop();
    std::cout << "\n=== Multi-Currency Simulation Completed ===\n";
    
    return 0;
}
