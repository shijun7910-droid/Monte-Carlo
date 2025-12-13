#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>
#include "include/MonteCarlo.h"
#include "include/CurrencyModel.h"
#include "include/RandomGenerator.h"
#include "include/Statistics.h"

void runGBMSimulation() {
    std::cout << "=== GBM Model Simulation USD/EUR Exchange Rate ===" << std::endl;
    
    // Parameters
    double initialRate = 0.92;  // Initial exchange rate
    double mu = 0.02;           // Annual return rate 2%
    double sigma = 0.15;        // Annual volatility 15%
    int numSimulations = 10000;
    int timeSteps = 252;        // Trading days in a year
    double timeHorizon = 1.0;   // 1 year
    
    // Create model and random number generator
    auto model = std::make_unique<GBM>(mu, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(42);
    
    // Create simulator
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, timeSteps, timeHorizon);
    
    // Run simulation
    auto results = simulator.runSimulation(initialRate);
    
    // Calculate risk metrics
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    // Output results
    std::cout << "\nSimulation Results Statistics:" << std::endl;
    std::cout << "Initial Rate: " << initialRate << std::endl;
    std::cout << "Number of Simulations: " << numSimulations << std::endl;
    std::cout << "Time Steps: " << timeSteps << std::endl;
    std::cout << "Time Horizon: " << timeHorizon << " years" << std::endl;
    std::cout << "\nFinal Rate Distribution:" << std::endl;
    std::cout << "Mean: " << std::fixed << std::setprecision(4) << metrics.mean << std::endl;
    std::cout << "Standard Deviation: " << metrics.standardDeviation << std::endl;
    std::cout << "Minimum: " << metrics.minValue << std::endl;
    std::cout << "Maximum: " << metrics.maxValue << std::endl;
    std::cout << "Median: " << metrics.median << std::endl;
    std::cout << "95% VaR: " << metrics.var95 << std::endl;
    std::cout << "95% CVaR: " << metrics.cvar95 << std::endl;
    
    // Save some paths to CSV
    std::ofstream file("gbm_paths.csv");
    if (file.is_open()) {
        // Write time points
        file << "Time";
        for (int i = 0; i < 10 && i < results.paths.size(); i++) {
            file << ",Path_" << i;
        }
        file << "\n";
        
        // Write data
        for (int t = 0; t < timeSteps; t++) {
            file << results.timePoints[t];
            for (int i = 0; i < 10 && i < results.paths.size(); i++) {
                file << "," << results.paths[i][t];
            }
            file << "\n";
        }
        file.close();
        std::cout << "\nPath data saved to gbm_paths.csv" << std::endl;
    }
}

void runVasicekSimulation() {
    std::cout << "\n=== Vasicek Model Simulation Interest Rate ===" << std::endl;
    
    // Parameters
    double initialRate = 0.05;  // Initial interest rate 5%
    double kappa = 0.1;         // Mean reversion speed
    double theta = 0.05;        // Long-term mean
    double sigma = 0.02;        // Volatility
    int numSimulations = 5000;
    
    auto model = std::make_unique<Vasicek>(kappa, theta, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(123);
    
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, 252, 1.0);
    
    auto results = simulator.runSimulation(initialRate);
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    std::cout << "\nVasicek Model Results:" << std::endl;
    std::cout << "Mean: " << metrics.mean << " (Long-term mean: " << theta << ")" << std::endl;
    std::cout << "Standard Deviation: " << metrics.standardDeviation << std::endl;
}

void saveFinalValuesToCSV(const std::vector<double>& finalValues, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Final_Values\n";
        for (double value : finalValues) {
            file << value << "\n";
        }
        file.close();
        std::cout << "Final values saved to " << filename << std::endl;
    }
}

int main() {
    std::cout << "Currency Exchange Rate Monte Carlo Simulation System\n" << std::endl;
    
    try {
        runGBMSimulation();
        runVasicekSimulation();
        
        std::cout << "\nSimulation completed successfully!" << std::endl;
        std::cout << "\nTo visualize results, run: python scripts/plot_results.py" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
