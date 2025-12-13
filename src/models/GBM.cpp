#include "models/GBM.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

GBM::GBM(double initialPrice, double drift, double volatility)
    : S0_(initialPrice), mu_(drift), sigma_(volatility) {
    
    if (!validateParameters()) {
        throw std::invalid_argument("Invalid GBM parameters");
    }
}

std::string GBM::getName() const {
    return "Geometric Brownian Motion";
}

double GBM::simulate(double S0, double dt, double random) const {
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    
    return S0 * std::exp((mu_ - 0.5 * sigma_ * sigma_) * dt 
                       + sigma_ * std::sqrt(dt) * random);
}

std::vector<double> GBM::simulatePath(
    double S0, 
    int steps, 
    double dt,
    const std::vector<double>& randoms
) const {
    if (steps <= 0) {
        throw std::invalid_argument("Number of steps must be positive");
    }
    
    if (static_cast<size_t>(steps) != randoms.size()) {
        throw std::invalid_argument("Random vector size must match number of steps");
    }
    
    std::vector<double> path(steps);
    double current = S0;
    
    for (int i = 0; i < steps; ++i) {
        current = simulate(current, dt, randoms[i]);
        path[i] = current;
    }
    
    return path;
}

double GBM::getInitialPrice() const {
    return S0_;
}

double GBM::getDrift() const {
    return mu_;
}

double GBM::getVolatility() const {
    return sigma_;
}

void GBM::setParameters(const std::vector<double>& params) {
    if (params.size() >= 3) {
        S0_ = params[0];
        mu_ = params[1];
        sigma_ = params[2];
        
        if (!validateParameters()) {
            throw std::invalid_argument("Invalid GBM parameters");
        }
    }
}

std::vector<double> GBM::getParameters() const {
    return {S0_, mu_, sigma_};
}

bool GBM::validateParameters() const {
    if (S0_ <= 0.0) {
        std::cerr << "Error: Initial price must be positive" << std::endl;
        return false;
    }
    if (sigma_ < 0.0) {
        std::cerr << "Error: Volatility cannot be negative" << std::endl;
        return false;
    }
    return true;
}

double GBM::expectedValue(double t) const {
    return S0_ * std::exp(mu_ * t);
}

double GBM::variance(double t) const {
    return S0_ * S0_ * std::exp(2 * mu_ * t) * (std::exp(sigma_ * sigma_ * t) - 1);
}
