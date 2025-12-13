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
        {"Bear Market", baseModel->getDrift() - 0.02, baseModel->getVolatility() + 0.1},
        {"High Volatility", baseModel->getDrift(), baseModel->getVolatility() + 0.15},
        {"Low Volatility", baseModel->getDrift(), std::max(0.05, baseModel->getVolatility() - 0.1)}
    };
    
    std::vector<std::vector<std::string>> scenarioResults;
    
    for (const auto& scenario : scenarios) {
        auto scenarioModel = std::make_shared<GBM>(
            baseModel->getInitialPrice(),
            scenario.drift,
            scenario.volatility
        );
        
        auto simulator = std::make_shared<MonteCarloSimulator>(scenarioModel);
        simulator->setSeed(42);
        
        auto result = simulator->runSimulation(10000, 252, 1.0/252.0);
        
        // Calculate probability of loss > 10%
        int lossCount = 0;
        for (double ret : result.returns) {
            if (ret < -0.10) lossCount++;
        }
        double lossProbability = static_cast<double>(lossCount) / result.returns.size();
        
        scenarioResults.push_back({
            scenario.name,
            std::to_string(scenario.drift),
            std::to_string(scenario.volatility),
            std::to_string(result.priceSummary.mean),
            std::to_string(result.returnSummary.stdDev * std::sqrt(252)),
            std::to_string(lossProbability)
        });
        
        std::cout << std::setw(15) << scenario.name
                  << " | Drift: " << std::setw(5) << scenario.drift * 100 << "%"
                  << " | Vol: " << std::setw(5) << scenario.volatility * 100 << "%"
                  << " | Mean: " << std::setw(8) << result.priceSummary.mean
                  << " | P(Loss>10%): " << std::setw(6) << lossProbability * 100 << "%\n";
    }
    
    // Save scenario analysis results
    std::string filename = currencyName + "_scenario_analysis.csv";
    std::replace(filename.begin(), filename.end(), '/', '_');
    CSVWriter::write(scenarioResults, filename,
                    {"Scenario", "Drift", "Volatility", "MeanPrice", "AnnualVol", "LossProbability"});
}

int main() {
    // Setup logger
    Logger::getInstance().setConsoleOutput(true);
    Logger::getInstance().setMinLevel(LogLevel::INFO);
    
    std::cout << "=== Comprehensive Risk Analysis ===\n\n";
    
    Timer totalTimer("Total Risk Analysis");
    
    // Define test cases
    std::vector<std::pair<std::string, std::shared_ptr<StochasticModel>>> testCases = {
        {"USD/RUB", std::make_shared<GBM>(75.0, 0.05, 0.25)},
        {"EUR/USD", std::make_shared<GBM>(1.10, 0.02, 0.15)},
        {"High Risk", std::make_shared<GBM>(100.0, 0.10, 0.40)}
    };
    
    for (const auto& [currencyName, model] : testCases) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Analyzing: " << currencyName << "\n";
        std::cout << std::string(60, '=') << "\n";
        
        Timer caseTimer(currencyName + " analysis");
        
        // Run comprehensive simulation
        auto simulator = std::make_shared<MonteCarloSimulator>(model);
        simulator->setSeed(42);
        
        auto result = simulator->runSimulation(50000, 252, 1.0/252.0);
        
        // Create result analyzer
        ResultAnalyzer analyzer(result.finalPrices);
        
        // Print detailed analysis
        analyzer.printAnalysis();
        
        // Additional risk analysis
        std::cout << "\nAdditional Risk Metrics:\n";
        std::cout << std::string(30, '-') << "\n";
        
        // Calculate Value at Risk for different confidence levels
        std::vector<double> confidenceLevels = {0.90, 0.95, 0.99};
        for (double conf : confidenceLevels) {
            double var = RiskMetrics::calculateVaR(result.returns, conf);
            double cvar = RiskMetrics::calculateCVaR(result.returns, conf);
            std::cout << conf * 100 << "% VaR: " << var * 100 << "%, "
                      << conf * 100 << "% CVaR: " << cvar * 100 << "%\n";
        }
        
        // Calculate expected shortfalls
        auto shortfalls = analyzer.calculateExpectedShortfalls({0.95, 0.99});
        std::cout << "Expected Shortfall 95%: " << shortfalls[0] * 100 << "%\n";
        std::cout << "Expected Shortfall 99%: " << shortfalls[1] * 100 << "%\n";
        
        // Calculate distribution moments
        auto moments = analyzer.calculateMoments(4);
        std::cout << "\nDistribution Moments:\n";
        std::cout << "Mean: " << moments[0] << "\n";
        std::cout << "Variance: " << moments[1] << "\n";
        std::cout << "Skewness: " << moments[2] << "\n";
        std::cout << "Kurtosis: " << moments[3] << "\n";
        
        // Save comprehensive report
        std::string filename = currencyName + "_risk_report.csv";
        std::replace(filename.begin(), filename.end(), '/', '_');
        
        auto report = analyzer.generateReport();
        std::vector<std::string> headers = {
            "Price_Mean", "Price_Median", "Price_StdDev", "Price_Min", "Price_Max", 
            "Price_Skewness", "Price_Kurtosis",
            "Return_Mean", "Return_StdDev", "Return_Skewness", "Return_Kurtosis",
            "VaR_95", "CVaR_95", "Volatility", "Sharpe_Ratio", "Max_Drawdown"
        };
        
        // Flatten the report
        std::vector<std::vector<double>> flatReport;
        std::vector<double> row;
        for (const auto& section : report) {
            row.insert(row.end(), section.begin(), section.end());
        }
        flatReport.push_back(row);
        
        CSVWriter::write(flatReport, filename, headers);
        std::cout << "\nRisk report saved to: " << filename << "\n";
        
        // Run specialized analyses
        analyzeVaRConvergence(model, currencyName);
        stressTest(model, currencyName);
        scenarioAnalysis(model, currencyName);
        
        caseTimer.stop();
    }
    
    totalTimer.stop();
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "=== Risk Analysis Completed ===\n";
    std::cout << "Generated reports can be used for:\n";
    std::cout << "1. Regulatory compliance reporting\n";
    std::cout << "2. Risk management decisions\n";
    std::cout << "3. Portfolio optimization\n";
    std::cout << "4. Stress testing and scenario analysis\n";
    
    return 0;
}
