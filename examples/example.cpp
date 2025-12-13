/**
 * Example program for Currency Exchange Rate Monte Carlo Simulation
 * 
 * This example demonstrates various features of the simulation library:
 * 1. Basic GBM simulation
 * 2. Vasicek model simulation
 * 3. Multiple simulations with different parameters
 * 4. Statistical analysis and risk metrics
 * 5. Exporting results to files
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <memory>
#include <cmath>
#include "include/MonteCarlo.h"
#include "include/CurrencyModel.h"
#include "include/RandomGenerator.h"
#include "include/Statistics.h"

// Function prototypes
void example1_basic_gbm();
void example2_vasicek_model();
void example3_multiple_simulations();
void example4_risk_analysis();
void example5_export_results();

int main() {
    std::cout << "=====================================================\n";
    std::cout << "Currency Exchange Rate Monte Carlo Simulation Examples\n";
    std::cout << "=====================================================\n\n";
    
    int choice;
    do {
        std::cout << "Select an example to run:\n";
        std::cout << "1. Basic GBM Simulation\n";
        std::cout << "2. Vasicek Model Simulation\n";
        std::cout << "3. Multiple Simulations Comparison\n";
        std::cout << "4. Risk Analysis with Different Parameters\n";
        std::cout << "5. Export Results to Files\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        
        switch(choice) {
            case 1:
                example1_basic_gbm();
                break;
            case 2:
                example2_vasicek_model();
                break;
            case 3:
                example3_multiple_simulations();
                break;
            case 4:
                example4_risk_analysis();
                break;
            case 5:
                example5_export_results();
                break;
            case 0:
                std::cout << "Exiting example program.\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
        std::cout << "\n";
    } while (choice != 0);
    
    return 0;
}

/**
 * Example 1: Basic GBM Simulation
 * Demonstrates a simple simulation with Geometric Brownian Motion
 */
void example1_basic_gbm() {
    std::cout << "\n=== Example 1: Basic GBM Simulation ===\n";
    
    // Parameters for USD/EUR exchange rate
    double initialRate = 0.92;  // 1 USD = 0.92 EUR
    double mu = 0.03;           // 3% annual expected return
    double sigma = 0.12;        // 12% annual volatility
    int numSimulations = 5000;
    int timeSteps = 252;        // Daily steps for 1 year
    double timeHorizon = 1.0;   // 1 year simulation
    
    // Create model and random generator
    auto model = std::make_unique<GBM>(mu, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(12345);
    
    // Create and run simulator
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, timeSteps, timeHorizon);
    
    auto results = simulator.runSimulation(initialRate);
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    // Display results
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nSimulation Parameters:\n";
    std::cout << "-----------------------\n";
    std::cout << "Initial exchange rate: " << initialRate << " EUR/USD\n";
    std::cout << "Expected return (mu): " << mu << " (" << mu*100 << "%)\n";
    std::cout << "Volatility (sigma): " << sigma << " (" << sigma*100 << "%)\n";
    std::cout << "Time horizon: " << timeHorizon << " years\n";
    std::cout << "Number of simulations: " << numSimulations << "\n";
    
    std::cout << "\nResults after " << timeHorizon << " year(s):\n";
    std::cout << "--------------------------------\n";
    std::cout << "Expected exchange rate: " << metrics.mean << "\n";
    std::cout << "Standard deviation: " << metrics.standardDeviation << "\n";
    std::cout << "Minimum simulated rate: " << metrics.minValue << "\n";
    std::cout << "Maximum simulated rate: " << metrics.maxValue << "\n";
    std::cout << "Median rate: " << metrics.median << "\n";
    std::cout << "95% Value at Risk (VaR): " << metrics.var95 << "\n";
    std::cout << "95% Conditional VaR (CVaR): " << metrics.cvar95 << "\n";
    
    // Calculate probability of rate increase
    int countIncrease = 0;
    for (double rate : results.finalValues) {
        if (rate > initialRate) countIncrease++;
    }
    double probIncrease = static_cast<double>(countIncrease) / numSimulations * 100;
    std::cout << "Probability of rate increase: " << probIncrease << "%\n";
}

/**
 * Example 2: Vasicek Model Simulation
 * Demonstrates mean reversion behavior
 */
void example2_vasicek_model() {
    std::cout << "\n=== Example 2: Vasicek Model Simulation ===\n";
    
    // Parameters for interest rate simulation
    double initialRate = 0.04;  // 4% initial interest rate
    double kappa = 0.5;         // Mean reversion speed
    double theta = 0.05;        // Long-term mean interest rate (5%)
    double sigma = 0.02;        // Volatility
    int numSimulations = 3000;
    int timeSteps = 252;        // Daily steps for 1 year
    double timeHorizon = 2.0;   // 2 year simulation
    
    auto model = std::make_unique<Vasicek>(kappa, theta, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(54321);
    
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, timeSteps, timeHorizon);
    
    auto results = simulator.runSimulation(initialRate);
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nVasicek Model Parameters:\n";
    std::cout << "--------------------------\n";
    std::cout << "Initial interest rate: " << initialRate * 100 << "%\n";
    std::cout << "Long-term mean (theta): " << theta * 100 << "%\n";
    std::cout << "Mean reversion speed (kappa): " << kappa << "\n";
    std::cout << "Volatility (sigma): " << sigma << "\n";
    
    std::cout << "\nSimulation Results:\n";
    std::cout << "-------------------\n";
    std::cout << "Mean interest rate after " << timeHorizon << " years: " 
              << metrics.mean * 100 << "%\n";
    std::cout << "Standard deviation: " << metrics.standardDeviation * 100 << "%\n";
    std::cout << "Range: " << metrics.minValue * 100 << "% to " 
              << metrics.maxValue * 100 << "%\n";
    
    // Check if mean reversion is working
    double distanceFromMean = std::abs(metrics.mean - theta);
    std::cout << "Distance from long-term mean: " << distanceFromMean * 100 << "%\n";
    
    // Calculate probability of staying within bounds
    double lowerBound = theta * 0.9;  // 10% below long-term mean
    double upperBound = theta * 1.1;  // 10% above long-term mean
    int countWithinBounds = 0;
    for (double rate : results.finalValues) {
        if (rate >= lowerBound && rate <= upperBound) {
            countWithinBounds++;
        }
    }
    double probWithinBounds = static_cast<double>(countWithinBounds) / numSimulations * 100;
    std::cout << "Probability within ±10% of long-term mean: " 
              << probWithinBounds << "%\n";
}

/**
 * Example 3: Multiple Simulations with Different Parameters
 * Compares different volatility scenarios
 */
void example3_multiple_simulations() {
    std::cout << "\n=== Example 3: Multiple Simulations Comparison ===\n";
    
    struct SimulationConfig {
        std::string name;
        double mu;
        double sigma;
        int color;  // For visualization purposes
    };
    
    std::vector<SimulationConfig> scenarios = {
        {"Low Volatility", 0.02, 0.08, 1},
        {"Medium Volatility", 0.02, 0.15, 2},
        {"High Volatility", 0.02, 0.25, 3},
        {"High Return, Low Vol", 0.05, 0.10, 4},
        {"Low Return, High Vol", -0.01, 0.20, 5}
    };
    
    double initialRate = 1.0;
    int numSimulations = 2000;
    int timeSteps = 180;  // 6 months (approx)
    double timeHorizon = 0.5;  // 0.5 years
    
    std::cout << "\nComparing " << scenarios.size() << " different scenarios:\n";
    std::cout << "Initial rate: " << initialRate << "\n";
    std::cout << "Time horizon: " << timeHorizon << " years\n";
    std::cout << "Number of simulations per scenario: " << numSimulations << "\n\n";
    
    std::cout << std::left << std::setw(25) << "Scenario" 
              << std::setw(15) << "Mu" 
              << std::setw(15) << "Sigma" 
              << std::setw(15) << "Mean Result" 
              << std::setw(15) << "Std Dev" 
              << std::setw(15) << "95% VaR\n";
    std::cout << std::string(100, '-') << "\n";
    
    std::vector<std::vector<double>> allFinalValues;
    
    for (const auto& scenario : scenarios) {
        auto model = std::make_unique<GBM>(scenario.mu, scenario.sigma);
        auto randomGen = std::make_unique<MersenneTwister>(1000 + scenario.color);
        
        MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                      numSimulations, timeSteps, timeHorizon);
        
        auto results = simulator.runSimulation(initialRate);
        auto metrics = simulator.calculateRiskMetrics(results.finalValues);
        
        std::cout << std::left << std::setw(25) << scenario.name
                  << std::fixed << std::setprecision(4)
                  << std::setw(15) << scenario.mu
                  << std::setw(15) << scenario.sigma
                  << std::setw(15) << metrics.mean
                  << std::setw(15) << metrics.standardDeviation
                  << std::setw(15) << metrics.var95 << "\n";
        
        allFinalValues.push_back(results.finalValues);
    }
    
    // Find best and worst performing scenarios
    if (!allFinalValues.empty()) {
        std::vector<double> scenarioMeans;
        for (const auto& values : allFinalValues) {
            scenarioMeans.push_back(Statistics::mean(values));
        }
        
        auto maxIt = std::max_element(scenarioMeans.begin(), scenarioMeans.end());
        auto minIt = std::min_element(scenarioMeans.begin(), scenarioMeans.end());
        
        int bestIdx = std::distance(scenarioMeans.begin(), maxIt);
        int worstIdx = std::distance(scenarioMeans.begin(), minIt);
        
        std::cout << "\nSummary:\n";
        std::cout << "Best performing scenario: " << scenarios[bestIdx].name 
                  << " (mean: " << scenarioMeans[bestIdx] << ")\n";
        std::cout << "Worst performing scenario: " << scenarios[worstIdx].name 
                  << " (mean: " << scenarioMeans[worstIdx] << ")\n";
    }
}

/**
 * Example 4: Risk Analysis with Different Confidence Levels
 */
void example4_risk_analysis() {
    std::cout << "\n=== Example 4: Risk Analysis ===\n";
    
    double initialRate = 100.0;  // JPY/USD (100 JPY = 1 USD)
    double mu = 0.01;            // 1% expected return
    double sigma = 0.18;         // 18% volatility
    int numSimulations = 10000;
    
    auto model = std::make_unique<GBM>(mu, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(999);
    
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, 252, 1.0);
    
    auto results = simulator.runSimulation(initialRate);
    auto finalValues = results.finalValues;
    
    // Calculate risk metrics for different confidence levels
    std::vector<double> confidenceLevels = {0.90, 0.95, 0.99};
    
    std::cout << "\nRisk Analysis for JPY/USD Exchange Rate:\n";
    std::cout << "Initial rate: " << initialRate << " JPY/USD\n";
    std::cout << "Number of simulations: " << numSimulations << "\n\n";
    
    std::cout << std::left << std::setw(15) << "Confidence"
              << std::setw(15) << "VaR"
              << std::setw(15) << "CVaR"
              << std::setw(25) << "VaR as % of Initial"
              << std::setw(25) << "Worst Case Loss\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (double confidence : confidenceLevels) {
        double var = Statistics::valueAtRisk(finalValues, confidence);
        double cvar = Statistics::conditionalVaR(finalValues, confidence);
        double varPercent = (var - initialRate) / initialRate * 100;
        double worstCaseLoss = (cvar - initialRate) / initialRate * 100;
        
        std::cout << std::fixed << std::setprecision(4)
                  << std::left << std::setw(15) << confidence * 100 << "%"
                  << std::setw(15) << var
                  << std::setw(15) << cvar
                  << std::setw(25) << varPercent << "%"
                  << std::setw(25) << worstCaseLoss << "%\n";
    }
    
    // Calculate probability of significant moves
    std::cout << "\nProbability Analysis:\n";
    
    double threshold10 = initialRate * 1.10;  // 10% increase
    double threshold20 = initialRate * 1.20;  // 20% increase
    double threshold10_down = initialRate * 0.90;  // 10% decrease
    double threshold20_down = initialRate * 0.80;  // 20% decrease
    
    int count10up = 0, count20up = 0, count10down = 0, count20down = 0;
    
    for (double rate : finalValues) {
        if (rate >= threshold10) count10up++;
        if (rate >= threshold20) count20up++;
        if (rate <= threshold10_down) count10down++;
        if (rate <= threshold20_down) count20down++;
    }
    
    std::cout << "Probability of 10% increase: " 
              << static_cast<double>(count10up) / numSimulations * 100 << "%\n";
    std::cout << "Probability of 20% increase: " 
              << static_cast<double>(count20up) / numSimulations * 100 << "%\n";
    std::cout << "Probability of 10% decrease: " 
              << static_cast<double>(count10down) / numSimulations * 100 << "%\n";
    std::cout << "Probability of 20% decrease: " 
              << static_cast<double>(count20down) / numSimulations * 100 << "%\n";
}

/**
 * Example 5: Export Results to Various File Formats
 */
void example5_export_results() {
    std::cout << "\n=== Example 5: Export Results to Files ===\n";
    
    // Run simulation
    double initialRate = 1.35;  // CAD/USD
    double mu = 0.015;
    double sigma = 0.10;
    int numSimulations = 500;
    int timeSteps = 63;  // Quarterly (252/4)
    double timeHorizon = 0.25;  // 3 months
    
    auto model = std::make_unique<GBM>(mu, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(777);
    
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, timeSteps, timeHorizon);
    
    auto results = simulator.runSimulation(initialRate);
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    std::cout << "Running simulation for CAD/USD exchange rate...\n";
    
    // 1. Export paths to CSV
    std::ofstream pathsFile("simulation_paths.csv");
    if (pathsFile.is_open()) {
        pathsFile << "Time";
        for (int i = 0; i < 20 && i < results.paths.size(); i++) {
            pathsFile << ",Path_" << i;
        }
        pathsFile << "\n";
        
        for (int t = 0; t < timeSteps; t++) {
            pathsFile << results.timePoints[t];
            for (int i = 0; i < 20 && i < results.paths.size(); i++) {
                pathsFile << "," << results.paths[i][t];
            }
            pathsFile << "\n";
        }
        pathsFile.close();
        std::cout << "✓ Saved 20 sample paths to simulation_paths.csv\n";
    }
    
    // 2. Export final values to CSV
    std::ofstream valuesFile("final_values.csv");
    if (valuesFile.is_open()) {
        valuesFile << "Simulation,Final_Value,Return_Percent\n";
        for (int i = 0; i < results.finalValues.size(); i++) {
            double returnPercent = (results.finalValues[i] - initialRate) / initialRate * 100;
            valuesFile << i << "," << results.finalValues[i] << "," << returnPercent << "\n";
        }
        valuesFile.close();
        std::cout << "✓ Saved all final values to final_values.csv\n";
    }
    
    // 3. Export summary statistics to CSV
    std::ofstream statsFile("summary_statistics.csv");
    if (statsFile.is_open()) {
        statsFile << "Statistic,Value\n";
        statsFile << "Initial_Rate," << initialRate << "\n";
        statsFile << "Mu," << mu << "\n";
        statsFile << "Sigma," << sigma << "\n";
        statsFile << "Time_Horizon," << timeHorizon << "\n";
        statsFile << "Number_Simulations," << numSimulations << "\n";
        statsFile << "Mean_Final_Rate," << metrics.mean << "\n";
        statsFile << "Std_Dev," << metrics.standardDeviation << "\n";
        statsFile << "Minimum," << metrics.minValue << "\n";
        statsFile << "Maximum," << metrics.maxValue << "\n";
        statsFile << "Median," << metrics.median << "\n";
        statsFile << "VaR_95," << metrics.var95 << "\n";
        statsFile << "CVaR_95," << metrics.cvar95 << "\n";
        
        // Add percentiles
        std::vector<double> percentiles = {0.01, 0.05, 0.10, 0.25, 0.75, 0.90, 0.95, 0.99};
        for (double p : percentiles) {
            double value = Statistics::percentile(results.finalValues, p);
            statsFile << "Percentile_" << p * 100 << "," << value << "\n";
        }
        statsFile.close();
        std::cout << "✓ Saved summary statistics to summary_statistics.csv\n";
    }
    
    // 4. Export for Python visualization
    std::ofstream pyFile("for_python_visualization.csv");
    if (pyFile.is_open()) {
        pyFile << "path_id,time,value\n";
        for (int i = 0; i < 50 && i < results.paths.size(); i++) {
            for (int t = 0; t < timeSteps; t++) {
                pyFile << i << "," << results.timePoints[t] << "," << results.paths[i][t] << "\n";
            }
        }
        pyFile.close();
        std::cout << "✓ Saved data for Python visualization to for_python_visualization.csv\n";
    }
    
    // 5. Create a simple report
    std::ofstream reportFile("simulation_report.txt");
    if (reportFile.is_open()) {
        reportFile << "MONTE CARLO SIMULATION REPORT\n";
        reportFile << "==============================\n\n";
        reportFile << "Simulation Date: " << __DATE__ << "\n";
        reportFile << "Currency Pair: CAD/USD\n\n";
        
        reportFile << "PARAMETERS\n";
        reportFile << "----------\n";
        reportFile << "Initial Rate: " << initialRate << "\n";
        reportFile << "Expected Return (mu): " << mu << " (" << mu*100 << "%)\n";
        reportFile << "Volatility (sigma): " << sigma << " (" << sigma*100 << "%)\n";
        reportFile << "Time Horizon: " << timeHorizon << " years\n";
        reportFile << "Number of Simulations: " << numSimulations << "\n\n";
        
        reportFile << "RESULTS\n";
        reportFile << "-------\n";
        reportFile << "Mean Final Rate: " << metrics.mean << "\n";
        reportFile << "Standard Deviation: " << metrics.standardDeviation << "\n";
        reportFile << "Range: " << metrics.minValue << " to " << metrics.maxValue << "\n";
        reportFile << "95% Value at Risk: " << metrics.var95 << "\n";
        reportFile << "95% Conditional VaR: " << metrics.cvar95 << "\n\n";
        
        // Calculate expected return
        double expectedReturn = (metrics.mean - initialRate) / initialRate * 100;
        reportFile << "Expected Return: " << expectedReturn << "%\n";
        
        // Risk-adjusted return (simplified Sharpe ratio)
        double riskFreeRate = 0.02;  // Assume 2% risk-free rate
        double excessReturn = (metrics.mean / initialRate - 1) - riskFreeRate * timeHorizon;
        double sharpeRatio = excessReturn / metrics.standardDeviation;
        reportFile << "Sharpe Ratio (approx): " << sharpeRatio << "\n";
        
        reportFile.close();
        std::cout << "✓ Generated simulation report in simulation_report.txt\n";
    }
    
    std::cout << "\nAll files exported successfully!\n";
    std::cout << "You can now visualize the results using Python scripts.\n";
}
