#ifndef MONTE_CARLO_SIMULATOR_H
#define MONTE_CARLO_SIMULATOR_H

#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "models/StochasticModel.h"
#include "random/RandomGenerator.h"
#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"

#ifdef _OPENMP
#include <omp.h>
#endif

struct SimulationResult {
    std::vector<std::vector<double>> paths;
    std::vector<double> finalPrices;
    std::vector<double> returns;
    double executionTime = 0.0;
    StatisticalSummary priceSummary;
    StatisticalSummary returnSummary;
    
    void print() const {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n=== Simulation Results ===\n";
        std::cout << "Number of paths: " << paths.size() << "\n";
        if (!paths.empty()) {
            std::cout << "Steps per path: " << paths[0].size() << "\n";
        }
        std::cout << "Execution time: " << executionTime << " seconds\n";
        std::cout << "Simulations per second: " 
                  << paths.size() / executionTime << "\n";
        
        priceSummary.print();
    }
};

class MonteCarloSimulator {
private:
    std::shared_ptr<StochasticModel> model_;
    std::shared_ptr<RandomGenerator> randomGenerator_;
    unsigned int seed_;
    int numThreads_;
    
public:
    MonteCarloSimulator(std::shared_ptr<StochasticModel> model, 
                       std::shared_ptr<RandomGenerator> randomGen = nullptr)
        : model_(model), randomGenerator_(randomGen), seed_(12345), numThreads_(1) {
        
        if (!randomGenerator_) {
            randomGenerator_ = std::make_shared<NormalGenerator>(0.0, 1.0, seed_);
        }
        
#ifdef _OPENMP
        numThreads_ = omp_get_max_threads();
#endif
    }
    
    void setSeed(unsigned int seed) {
        seed_ = seed;
        if (randomGenerator_) {
            randomGenerator_->setSeed(seed);
        }
    }
    
    void setNumThreads(int threads) {
        numThreads_ = threads;
    }
    
    void setModel(std::shared_ptr<StochasticModel> model) {
        model_ = model;
    }
    
    void setRandomGenerator(std::shared_ptr<RandomGenerator> generator) {
        randomGenerator_ = generator;
    }
    
    SimulationResult runSimulation(
        int numPaths,
        int steps,
        double dt
    ) {
        auto start = std::chrono::high_resolution_clock::now();
        
        SimulationResult result;
        result.paths.resize(numPaths);
        result.finalPrices.resize(numPaths);
        result.returns.resize(numPaths);
        
        double initialPrice = model_->getInitialPrice();
        
#ifdef _OPENMP
        omp_set_num_threads(numThreads_);
        #pragma omp parallel for
#endif
        for (int i = 0; i < numPaths; ++i) {
            std::vector<double> randoms = randomGenerator_->generateVector(steps);
            result.paths[i] = model_->simulatePath(initialPrice, steps, dt, randoms);
            result.finalPrices[i] = result.paths[i].back();
            result.returns[i] = (result.finalPrices[i] - initialPrice) / initialPrice;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double>(end - start).count();
        
        result.priceSummary = Statistics::analyze(result.finalPrices);
        result.returnSummary = Statistics::analyze(result.returns);
        
        return result;
    }
    
    SimulationResult runSimulationBatch(
        int numPaths,
        int steps,
        double dt,
        int batchSize = 1000
    ) {
        auto start = std::chrono::high_resolution_clock::now();
        
        SimulationResult result;
        result.finalPrices.resize(numPaths);
        result.returns.resize(numPaths);
        
        int numBatches = (numPaths + batchSize - 1) / batchSize;
        double initialPrice = model_->getInitialPrice();
        
#ifdef _OPENMP
        omp_set_num_threads(numThreads_);
#endif
        
        for (int batch = 0; batch < numBatches; ++batch) {
            int startIdx = batch * batchSize;
            int endIdx = std::min(startIdx + batchSize, numPaths);
            int currentBatchSize = endIdx - startIdx;
            
#ifdef _OPENMP
            #pragma omp parallel for
#endif
            for (int i = startIdx; i < endIdx; ++i) {
                std::vector<double> randoms = randomGenerator_->generateVector(steps);
                std::vector<double> path = model_->simulatePath(
                    initialPrice, steps, dt, randoms);
                result.finalPrices[i] = path.back();
                result.returns[i] = (result.finalPrices[i] - initialPrice) / initialPrice;
                
                // Store first few paths for analysis
                if (i < 100 && i < numPaths) {
                    if (result.paths.size() <= i) {
                        result.paths.push_back(path);
                    } else {
                        result.paths[i] = path;
                    }
                }
            }
            
            // Display progress
            if (batch % 10 == 0 || batch == numBatches - 1) {
                std::cout << "Progress: " << endIdx << "/" << numPaths 
                          << " (" << std::setprecision(1) << std::fixed
                          << (100.0 * endIdx / numPaths) << "%)" 
                          << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double>(end - start).count();
        
        result.priceSummary = Statistics::analyze(result.finalPrices);
        result.returnSummary = Statistics::analyze(result.returns);
        
        return result;
    }
    
    std::vector<double> calculateRiskMetrics(
        const std::vector<double>& returns,
        double confidence = 0.95
    ) {
        std::vector<double> metrics;
        
        double var = RiskMetrics::calculateVaR(returns, confidence);
        double cvar = RiskMetrics::calculateCVaR(returns, confidence);
        double volatility = RiskMetrics::calculateVolatility(returns);
        double sharpe = RiskMetrics::calculateSharpeRatio(returns);
        
        metrics.push_back(var);
        metrics.push_back(cvar);
        metrics.push_back(volatility);
        metrics.push_back(sharpe);
        
        return metrics;
    }
    
    std::vector<double> calculatePercentiles(
        const std::vector<double>& prices,
        const std::vector<double>& percentiles = {0.01, 0.05, 0.25, 0.5, 0.75, 0.95, 0.99}
    ) {
        std::vector<double> sorted = prices;
        std::sort(sorted.begin(), sorted.end());
        
        std::vector<double> results;
        for (double p : percentiles) {
            size_t index = static_cast<size_t>(p * sorted.size());
            if (index >= sorted.size()) index = sorted.size() - 1;
            results.push_back(sorted[index]);
        }
        
        return results;
    }
};

#endif // MONTE_CARLO_SIMULATOR_H
