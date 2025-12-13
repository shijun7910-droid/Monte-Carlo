#include "simulation/MonteCarloSimulator.h"
#include <iostream>
#include <iomanip>

MonteCarloSimulator::MonteCarloSimulator(
    std::shared_ptr<StochasticModel> model, 
    std::shared_ptr<RandomGenerator> randomGen)
    : model_(model), randomGenerator_(randomGen), seed_(12345), numThreads_(1) {
    
    if (!model_) {
        throw std::invalid_argument("Model cannot be null");
    }
    
    if (!randomGenerator_) {
        randomGenerator_ = std::make_shared<NormalGenerator>(0.0, 1.0, seed_);
    }
    
#ifdef _OPENMP
    numThreads_ = omp_get_max_threads();
#endif
}

void MonteCarloSimulator::setSeed(unsigned int seed) {
    seed_ = seed;
    if (randomGenerator_) {
        randomGenerator_->setSeed(seed);
    }
}

void MonteCarloSimulator::setNumThreads(int threads) {
    if (threads <= 0) {
        throw std::invalid_argument("Number of threads must be positive");
    }
    numThreads_ = threads;
}

void MonteCarloSimulator::setModel(std::shared_ptr<StochasticModel> model) {
    if (!model) {
        throw std::invalid_argument("Model cannot be null");
    }
    model_ = model;
}

void MonteCarloSimulator::setRandomGenerator(std::shared_ptr<RandomGenerator> generator) {
    if (!generator) {
        throw std::invalid_argument("Random generator cannot be null");
    }
    randomGenerator_ = generator;
}

SimulationResult MonteCarloSimulator::runSimulation(
    int numPaths,
    int steps,
    double dt
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    if (steps <= 0) {
        throw std::invalid_argument("Number of steps must be positive");
    }
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    SimulationResult result;
    result.paths.resize(numPaths);
    result.finalPrices.resize(numPaths);
    result.returns.resize(numPaths);
    
    double initialPrice = model_->getInitialPrice();
    
    // Pre-generate all random numbers for better performance
    std::vector<std::vector<double>> allRandoms(numPaths);
    for (int i = 0; i < numPaths; ++i) {
        allRandoms[i] = randomGenerator_->generateVector(steps);
    }
    
#ifdef _OPENMP
    omp_set_num_threads(numThreads_);
    #pragma omp parallel for
#endif
    for (int i = 0; i < numPaths; ++i) {
        result.paths[i] = model_->simulatePath(initialPrice, steps, dt, allRandoms[i]);
        result.finalPrices[i] = result.paths[i].back();
        result.returns[i] = (result.finalPrices[i] - initialPrice) / initialPrice;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.executionTime = std::chrono::duration<double>(end - start).count();
    
    result.priceSummary = Statistics::analyze(result.finalPrices);
    result.returnSummary = Statistics::analyze(result.returns);
    
    return result;
}

SimulationResult MonteCarloSimulator::runSimulationBatch(
    int numPaths,
    int steps,
    double dt,
    int batchSize
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    if (steps <= 0) {
        throw std::invalid_argument("Number of steps must be positive");
    }
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    if (batchSize <= 0) {
        throw std::invalid_argument("Batch size must be positive");
    }
    
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
                if (result.paths.size() <= static_cast<size_t>(i)) {
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

std::vector<double> MonteCarloSimulator::calculateRiskMetrics(
    const std::vector<double>& returns,
    double confidence
) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }
    if (confidence <= 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("Confidence level must be between 0 and 1");
    }
    
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

std::vector<double> MonteCarloSimulator::calculatePercentiles(
    const std::vector<double>& prices,
    const std::vector<double>& percentiles
) {
    if (prices.empty()) {
        throw std::invalid_argument("Prices vector cannot be empty");
    }
    
    std::vector<double> sorted = prices;
    std::sort(sorted.begin(), sorted.end());
    
    std::vector<double> results;
    for (double p : percentiles) {
        if (p < 0.0 || p > 1.0) {
            throw std::invalid_argument("Percentile must be between 0 and 1");
        }
        size_t index = static_cast<size_t>(p * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        results.push_back(sorted[index]);
    }
    
    return results;
}
