#ifndef MONTECARLO_H
#define MONTECARLO_H

#include <vector>
#include <memory>
#include "CurrencyModel.h"
#include "RandomGenerator.h"
#include "Statistics.h"

struct SimulationResults {
    std::vector<std::vector<double>> paths;
    std::vector<double> finalValues;
    std::vector<double> timePoints;
};

class MonteCarloSimulator {
private:
    std::unique_ptr<CurrencyModel> model;
    std::unique_ptr<RandomGenerator> randomGen;
    int numSimulations;
    int timeSteps;
    double timeHorizon;
    
public:
    MonteCarloSimulator(std::unique_ptr<CurrencyModel> model,
                       std::unique_ptr<RandomGenerator> randomGen,
                       int numSimulations = 10000,
                       int timeSteps = 252,
                       double timeHorizon = 1.0);
    
    SimulationResults runSimulation(double initialRate);
    RiskMetrics calculateRiskMetrics(const std::vector<double>& finalRates);
};

#endif // MONTECARLO_H
