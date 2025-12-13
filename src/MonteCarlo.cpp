#include "../include/MonteCarlo.h"
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
    
    // Initialize time points
    results.timePoints.resize(timeSteps);
    double dt = timeHorizon / timeSteps;
    for (int i = 0; i < timeSteps; ++i) {
        results.timePoints[i] = i * dt;
    }
    
    // Initialize path storage
    results.paths.resize(numSimulations);
    results.finalValues.resize(numSimulations);
    
    // Generate all random numbers
    auto allRandomNumbers = randomGen->generateNormalMatrix(numSimulations, timeSteps);
    
    // Generate all paths
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
