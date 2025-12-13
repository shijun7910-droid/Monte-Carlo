#include "../include/MonteCarlo.h"
#include <iostream>
#include <algorithm>

MonteCarloSimulator::MonteCarloSimulator(std::unique_ptr<CurrencyModel> model,
                       std::unique_ptr<RandomGenerator> randomGen,
                       int numSimulations,
                       int timeSteps,
                       double timeHorizon)
    : model(std::move(model)), randomGen(std::move(randomGen)),
      numSimulations(numSimulations), timeSteps(timeSteps), 
      timeHorizon(timeHorizon) {}

SimulationResults MonteCarloSimulator::runSimulation(double initialRate) {
    SimulationResults results;
    
    // 初始化时间点
    results.timePoints.resize(timeSteps);
    double dt = timeHorizon / timeSteps;
    for (int i = 0; i < timeSteps; ++i) {
        results.timePoints[i] = i * dt;
    }
    
    // 初始化路径存储
    results.paths.resize(numSimulations);
    results.finalValues.resize(numSimulations);
    
    // 生成所有随机数
    auto allRandomNumbers = randomGen->generateNormalMatrix(numSimulations, timeSteps);
    
    // 并行生成所有路径
    for (int sim = 0; sim < numSimulations; ++sim) {
        results.paths[sim] = model->generatePath(initialRate, timeHorizon, 
                                                timeSteps, allRandomNumbers[sim]);
        results.finalValues[sim] = results.paths[sim].back();
    }
    
    return results;
}

RiskMetrics MonteCarloSimulator::calculateRiskMetrics(const std::vector<double>& finalRates) {
    return Statistics::calculateMetrics(finalRates);
}
