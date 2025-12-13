#include "models/HullWhite.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

HullWhite::HullWhite(double initialRate, double speed, double volatility)
    : r0_(initialRate), a_(speed), sigma_(volatility) {
    
    if (!validateParameters()) {
        throw std::invalid_argument("Invalid Hull-White parameters");
    }
    
    // Default theta function (constant)
    theta_ = [](double t) { return 0.05; };
}

std::string HullWhite::getName() const {
    return "Hull-White Model";
}

double HullWhite::simulate(double S0, double dt, double random) const {
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    
    double t = 0.0;  // Simplified: assume starting from time 0
    return S0 + (theta_(t) - a_ * S0) * dt + sigma_ * std::sqrt(dt) * random;
}

double HullWhite::getInitialPrice() const {
    return r0_;
}

double HullWhite::getDrift() const {
    return theta_(0.0);  // Drift at time 0
}

double HullWhite::getVolatility() const {
    return sigma_;
}

double HullWhite::getMeanReversion() const {
    return a_;
}

void HullWhite::setThetaFunction(std::function<double(double)> func) {
    if (!func) {
        throw std::invalid_argument("Theta function cannot be null");
    }
    theta_ = func;
}

void HullWhite::setParameters(const std::vector<double>& params) {
    if (params.size() >= 3) {
        r0_ = params[0];
        a_ = params[1];
        sigma_ = params[2];
        
        if (!validateParameters()) {
            throw std::invalid_argument("Invalid Hull-White parameters");
        }
    }
}

std::vector<double> HullWhite::getParameters() const {
    return {r0_, a_, sigma_};
}

bool HullWhite::validateParameters() const {
    if (a_ < 0.0) {
        std::cerr << "Error: Mean reversion speed cannot be negative" << std::endl;
        return false;
    }
    if (sigma_ < 0.0) {
        std::cerr << "Error: Volatility cannot be negative" << std::endl;
        return false;
    }
    return true;
}

double HullWhite::expectedValue(double t) const {
    // For Hull-White with constant theta, the expected value is:
    // E[r(t)] = theta/a + (r0 - theta/a) * exp(-a*t)
    double theta_constant = theta_(0.0);
    return theta_constant / a_ + (r0_ - theta_constant / a_) * std::exp(-a_ * t);
}

double HullWhite::variance(double t) const {
    // Variance of Hull-White model
    return (sigma_ * sigma_) / (2 * a_) * (1 - std::exp(-2 * a_ * t));
}
