#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>

#include "models/GBM.h"
#include "models/Vasicek.h"
#include "simulation/MonteCarloSimulator.h"
#include "statistics/Statistics.h"
#include "utils/CSVWriter.h"
#include "utils/Timer.h"
#include "utils/Logger.h"

int main() {
    // Setup logger
    Logger::getInstance().setConsoleOutput(true);
    Logger::getInstance().setMinLevel(LogLevel::INFO);
    
    std::cout << "=== Basic Currency Simulation Examples ===\n\n";
    
    // Example 1: USD/RUB simulation using GBM
    {
        std::cout << "Example 1: USD/RUB Simulation (GBM Model)\n";
        std::cout << "=========================================\n";
        
        ScopedTimer timer("USD/RUB GBM Simulation");
        
        auto usd_rub = std::make_shared<GBM>(75.0, 0.05, 0.2);
        auto simulator = std::make_shared<MonteCarloSimulator>(usd_rub);
        simulator->setSeed(42);
        
        auto result = simulator->runSimulation(10000, 252, 1.0/252.0);
        
        std::cout << "\nResults:\n";
        std::cout << "Initial USD/RUB: 75.0\n";
        std::cout << "Expected after 1 year: " << result.priceSummary.mean << "\n";
        std::cout << "95% Confidence Interval: [" 
                  << result.priceSummary.confidenceInterval95.first << ", "
                  << result.priceSummary.confidenceInterval95.second << "]\n";
        
        // Calculate probability of USD/RUB > 85
        int count = 0;
        for (double price : result.finalPrices) {
            if (price > 85.0) count++;
        }
        double probability = static_cast<double>(count) / result.finalPrices.size();
        std::cout << "Probability of USD/RUB > 85: " << probability * 100 << "%\n";
        
        // Save results
        CSVWriter::writeVector(result.finalPrices, "usd_rub_gbm_results.csv", "Price");
        std::cout << "Results saved to usd_rub_gbm_results.csv\n";
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n\n";
    
    // Example 2: EUR/USD simulation using Vasicek model
    {
        std::cout << "Example 2: EUR/USD Simulation (Vasicek Model)\n";
        std::cout << "=============================================\n";
        
        ScopedTimer timer("EUR/USD Vasicek Simulation");
        
        auto eur_usd = std::make_shared<Vasicek>(1.10, 1.5, 1.10, 0.15);
        auto simulator = std::make_shared<MonteCarloSimulator>(eur_usd);
        simulator->setSeed(123);
        
        auto result = simulator->runSimulation(5000, 180, 1.0/252.0);
        
        std::cout << "\nResults:\n";
        std::cout << "Initial EUR/USD: 1.10\n";
        std::cout << "Expected after 180 days: " << result.priceSummary.mean << "\n";
        std::cout << "Volatility: " << result.returnSummary.stdDev * std::sqrt(252) * 100 << "%\n";
        
        // Risk metrics
        auto riskMetrics = simulator->calculateRiskMetrics(result.returns, 0.95);
        std::cout << "95% VaR: " << riskMetrics[0] * 100 << "%\n";
        std::cout << "95% CVaR: " << riskMetrics[1] * 100 << "%\n";
        
        // Save results
        CSVWriter::writeVector(result.finalPrices, "eur_usd_vasicek_results.csv", "Price");
        std::cout << "Results saved to eur_usd_vasicek_results.csv\n";
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n\n";
    
    // Example 3: Compare different models
    {
        std::cout << "Example 3: Model Comparison\n";
        std::cout << "============================\n";
        
        std::vector<std::shared_ptr<StochasticModel>> models = {
            std::make_shared<GBM>(100.0, 0.05, 0.2),
            std::make_shared<GBM>(100.0, 0.05, 0.3),
            std::make_shared<Vasicek>(100.0, 1.0, 100.0, 0.2)
        };
        
        std::vector<std::string> modelNames = {"GBM (σ=0.2)", "GBM (σ=0.3)", "Vasicek"};
        
        std::vector<std::vector<std::string>> comparisonTable;
        
        for (size_t i = 0; i < models.size(); ++i) {
            auto simulator = std::make_shared<MonteCarloSimulator>(models[i]);
            simulator->setSeed(42 + i);
            
            auto result = simulator->runSimulation(5000, 252, 1.0/252.0);
            
            comparisonTable.push_back({
                modelNames[i],
                std::to_string(result.priceSummary.mean),
                std::to_string(result.priceSummary.stdDev),
                std::to_string(result.returnSummary.stdDev * std::sqrt(252)),
                std::to_string(result.priceSummary.confidenceInterval95.first) + " - " +
                std::to_string(result.priceSummary.confidenceInterval95.second)
            });
        }
        
        std::cout << "\nModel Comparison (100 simulations each):\n";
        std::vector<std::string> headers = {"Model", "Mean", "Std Dev", "Ann. Vol", "95% CI"};
        Logger::getInstance().logTable(headers, comparisonTable);
    }
    
    std::cout << "\n=== Examples Completed Successfully ===\n";
    return 0;
}
