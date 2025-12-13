#include "../include/CurrencyModel.h"
#include <cmath>
#include <iostream>

GBM::GBM(double mu, double sigma) : mu(mu), sigma(sigma) {
    std::cout << "GBM Model created with mu=" << mu << ", sigma=" << sigma << std::endl;
}

std::vector<double> GBM::generatePath(double initialRate, 
                                      double timeHorizon,
                                      int steps,
                                      const std::vector<double>& randomNumbers) {
    std::vector<double> path(steps);
    double dt = timeHorizon / steps;
    double current = initialRate;
    
    for (int i = 0; i < steps; i++) {
        double dW = randomNumbers[i] * sqrt(dt);
        current = current * exp((mu - 0.5 * sigma * sigma) * dt + sigma * dW);
        path[i] = current;
    }
    
    return path;
}

Vasicek::Vasicek(double kappa, double theta, double sigma) 
    : kappa(kappa), theta(theta), sigma(sigma) {
    std::cout << "Vasicek Model created with kappa=" << kappa 
              << ", theta=" << theta << ", sigma=" << sigma << std::endl;
}

std::vector<double> Vasicek::generatePath(double initialRate,
                                          double timeHorizon,
                                          int steps,
                                          const std::vector<double>& randomNumbers) {
    std::vector<double> path(steps);
    double dt = timeHorizon / steps;
    double current = initialRate;
    
    for (int i = 0; i < steps; i++) {
        double dW = randomNumbers[i] * sqrt(dt);
        current = current + kappa * (theta - current) * dt + sigma * dW;
        path[i] = current;
    }
    
    return path;
}
