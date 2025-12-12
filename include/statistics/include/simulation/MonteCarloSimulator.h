#ifndef MONTE_CARLO_SIMULATOR_H
#define MONTE_CARLO_SIMULATOR_H

#include <memory>
#include <vector>
#include <chrono>
#include "models/StochasticModel.h"
#include "random/RandomGenerator.h"
#include "statistics/Statistics.h"
#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

struct SimulationResult {
    std::vector<std::vector<double>> paths;
    std::vector<double> finalPrices;
    double executionTime = 0.0;
    StatisticalSummary summary;
    
    void print() const {
        std::cout << "\nSimulation Results\n";
        std::cout << "==================\n";
        std::cout << "Number of paths: " << paths.size() << "\n";
        if (!paths.empty()) {
            std::cout << "Steps per path: " << paths[0].size() << "\n";
        }
        std::cout << "Execution time: " << executionTime << " seconds\n";
        Statistics::printSummary(summary);
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
        
        // 设置默认线程数
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
    
    SimulationResult runSimulation(
        int numPaths,
        int steps,
        double dt
    ) {
        auto start = std::chrono::high_resolution_clock::now();
        
        SimulationResult result;
        result.paths.resize(numPaths);
        result.finalPrices.resize(numPaths);
        
        // 生成所有随机数
        std::vector<std::vector<double>> allRandoms(numPaths);
        for (int i = 0; i < numPaths; ++i) {
            allRandoms[i] = randomGenerator_->generateVector(steps);
        }
        
        // 并行模拟路径
#ifdef _OPENMP
        omp_set_num_threads(numThreads_);
        #pragma omp parallel for
#endif
        for (int i = 0; i < numPaths; ++i) {
            result.paths[i] = model_->simulatePath(
                model_->getInitialPrice(),
                steps,
                dt,
                allRandoms[i]
            );
            result.finalPrices[i] = result.paths[i].back();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double>(end - start).count();
        
        // 计算统计摘要
        result.summary = Statistics::analyze(result.finalPrices);
        
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
        
        int numBatches = (numPaths + batchSize - 1) / batchSize;
        
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
                auto randoms = randomGenerator_->generateVector(steps);
                auto path = model_->simulatePath(
                    model_->getInitialPrice(),
                    steps,
                    dt,
                    randoms
                );
                result.finalPrices[i] = path.back();
            }
            
            // 显示进度
            if (batch % 10 == 0) {
                std::cout << "Progress: " << endIdx << "/" << numPaths 
                          << " (" << (100.0 * endIdx / numPaths) << "%)" 
                          << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double>(end - start).count();
        
        result.summary = Statistics::analyze(result.finalPrices);
        
        return result;
    }
    
    std::vector<double> calculatePercentiles(
        const std::vector<double>& prices,
        const std::vector<double>& percentiles
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
