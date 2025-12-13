#include "models/Vasicek.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

Vasicek::Vasicek(double initialRate, double speed, 
                 double longTermMean, double volatility)
    : r0_(initialRate), theta_(speed), mu_(longTermMean), sigma_(volatility) {
    
    if (!validateParameters()) {
        throw std::invalid_argument("Invalid Vasicek parameters");
    }
}

std::string Vasicek::getName() const {
    return "Vasicek Model";
}

double Vasicek::simulate(double S0, double dt, double random) const {
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    
    return S0 + theta_ * (mu_ - S0) * dt + sigma_ * std::sqrt(dt) * random;
}

double Vasicek::getInitialPrice() const {
    return r0_;
}

double Vasicek::getDrift() const override {
    return mu_;
}

double Vasicek::getVolatility() const override {
    return sigma_;
}

double Vasicek::getMeanReversion() const {
    return theta_;
}

double Vasicek::getLongTermMean() const {
    return mu_;
}

void Vasicek::setParameters(const std::vector<double>& params) {
    if (params.size() >= 4) {
        r0_ = params[0];
        theta_ = params[1];
        mu_ = params[2];
        sigma_ = params[3];
        
        if (!validateParameters()) {
            throw std::invalid_argument("Invalid Vasicek parameters");
        }
    }
}

std::vector<double> Vasicek::getParameters() const {
    return {r0_, theta_, mu_, sigma_};
}

bool Vasicek::validateParameters() const {
    if (theta_ < 0.0) {
        std::cerr << "Error: Mean reversion speed cannot be negative" << std::endl;
        return false;
    }
    if (sigma_ < 0.0) {
        std::cerr << "Error: Volatility cannot be negative" << std::endl;
        return false;
    }
    return true;
}

double Vasicek::expectedValue(double t) const {
    return mu_ + (r0_ - mu_) * std::exp(-theta_ * t);
}

double Vasicek::variance(double t) const {
    return (sigma_ * sigma_) / (2 * theta_) * (1 - std::exp(-2 * theta_ * t));
}
