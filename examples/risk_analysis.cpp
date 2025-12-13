#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "models/GBM.h"
#include "simulation/MonteCarloSimulator.h"
#include "simulation/ResultAnalyzer.h"
#include "statistics/RiskMetrics.h"
#include "utils/CSVWriter.h"
#include "utils/Timer.h"
#include "utils/Logger.h"

void analyzeVaRConvergence(std::shared_ptr<StochasticModel> model, 
                          const std::string& currencyName) {
    std::cout << "\nVaR Convergence Analysis for " << currencyName << ":\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::vector<int> sampleSizes = {100, 500, 1000, 5000, 10000, 50000};
    std::vector<double> var95Results;
    std::vector<double> var99Results;
    
    for (int n : sampleSizes) {
        auto simulator = std::make_shared<MonteCarloSimulator>(model);
        simulator->setSeed(42);
        
        auto result = simulator->runSimulation(n, 252, 1.0/252.0);
        
        double var95 = RiskMetrics::calculateVaR(result.returns, 0.95);
        double var99 = RiskMetrics::calculateVaR(result.returns, 0.99);
        
        var95Results.push_back(var95);
        var99Results.push_back(var99);
        
        std::cout << "n=" << std::setw(6) << n 
                  << " | 95% VaR: " << std::setw(8) << std::fixed << std::setprecision(4) 
                  << var95 * 100 << "%"
                  << " | 99% VaR: " << std::setw(8) << var99 * 100 << "%\n";
    }
    
    // Save convergence results
    std::vector<std::vector<std::string>> convergenceData;
    for (size_t i = 0; i < sampleSizes.size(); ++i) {
        convergenceData.push_back({
            std::to_string(sampleSizes[i]),
            std::to_string(var95Results[i]),
            std::to_string(var99Results[i])
        });
    }
    
    std::string filename = currencyName + "_var_convergence.csv";
    std::replace(filename.begin(), filename.end(), '/', '_');
    CSVWriter::write(convergenceData, filename, {"SampleSize", "VaR_95", "VaR_99"});
}

void stressTest(std::shared_ptr<StochasticModel> model,
               const std::string& currencyName) {
    std::cout << "\nStress Test for " << currencyName << ":\n";
    std::cout << std::string(50, '-') << "\n";
    
    std::vector<double> volatilityLevels = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<std::vector<std::string>> stressResults;
    
    for (double vol : volatilityLevels) {
        // Create model with different volatility
        auto stressModel = std::make_shared<GBM>(
            model->getInitialPrice(),
            model->getDrift(),
            vol
        );
        
        auto simulator = std::make_shared<MonteCarloSimulator>(stressModel);
        simulator->setSeed(42);
        
        auto result = simulator->runSimulation(10000, 252, 1.0/252.0);
        auto riskMetrics = simulator->calculateRiskMetrics(result.returns, 0.95);
        
        stressResults.push_back({
            std::to_string(vol),
            std::to_string(result.priceSummary.mean),
            std::to_string(result.returnSummary.stdDev * std::sqrt(252)),
            std::to_string(riskMetrics[0]),  // VaR
            std::to_string(riskMetrics[1]),  // CVaR
            std::to_string(riskMetrics[3])   // Sharpe Ratio
        });
        
        std::cout << "Volatility: " << vol * 100 << "%"
                  << " | Mean: " << result.priceSummary.mean
                  << " | Ann. Vol: " << result.returnSummary.stdDev * std::sqrt(252) * 100 << "%"
                  << " | VaR: " << riskMetrics[0] * 100 << "%"
                  << " | Sharpe: " << riskMetrics[3] << "\n";
    }
    
    // Save stress test results
    std::string filename = currencyName + "_stress_test.csv";
    std::replace(filename.begin(), filename.end(), '/', '_');
    CSVWriter::write(stressResults, filename, 
                    {"Volatility", "MeanPrice", "AnnualVol", "VaR_95", "CVaR_95", "SharpeRatio"});
}

void scenarioAnalysis(std::shared_ptr<StochasticModel> baseModel,
                     const std::string& currencyName) {
    std::cout << "\nScenario Analysis for " << currencyName << ":\n";
    std::cout << std::string(50, '-') << "\n";
    
    struct Scenario {
        std::string name;
        double drift;
        double volatility;
    };
    
    std::vector<Scenario> scenarios = {
        {"Baseline", baseModel->getDrift(), baseModel->getVolatility()},
        {"Bull Market", baseModel->getDrift() + 0.02, baseModel->getVolatility()},
        {"Bear Market", baseModel->getDrift() - 0.02, baseModel->getVolatility()
