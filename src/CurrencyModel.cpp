#include "CurrencyModel.h"
#include <cmath>

// GBM 实现
GBM::GBM(double mu, double sigma) : mu(mu), sigma(sigma) {}

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

double GBM::drift(double rate, double time) {
    return mu * rate;
}

double GBM::diffusion(double rate, double time) {
    return sigma * rate;
}

// Vasicek 实现
Vasicek::Vasicek(double kappa, double theta, double sigma) 
    : kappa(kappa), theta(theta), sigma(sigma) {}

std::vector<double> Vasicek::generatePath(double initialRate,
                                          double timeHorizon,
                                          int steps,
                                          const std::vector<double>& randomNumbers) {
    std::vector<double> path(steps);
    double dt = timeHorizon / steps;
    double current = initialRate;
    
    for (int i = 0; i < steps; i++) {
        double dW = randomNumbers[i] * sqrt(dt);
        // Euler-Maruyama 方法
        current = current + kappa * (theta - current) * dt + sigma * dW;
        path[i] = current;
    }
    
    return path;
}

double Vasicek::drift(double rate, double time) {
    return kappa * (theta - rate);
}

double Vasicek::diffusion(double rate, double time) {
    return sigma;
}
