#ifndef HULLWHITE_H
#define HULLWHITE_H

#include "StochasticModel.h"
#include <string>
#include <vector>
#include <functional>

class HullWhite : public StochasticModel {
private:
    double a_;           // Mean reversion speed
    double sigma_;       // Volatility
    double r0_;          // Initial interest rate
    std::function<double(double)> theta_;  // Time-dependent function
    
public:
    HullWhite(double initialRate = 0.05, double speed = 0.1, 
              double volatility = 0.01)
        : r0_(initialRate), a_(speed), sigma_(volatility) {
        // Default theta function (constant)
        theta_ = [](double t) { return 0.05; };
    }
    
    std::string getName() const override {
        return "Hull-White Model";
    }
    
    double simulate(double S0, double dt, double random) const override {
        double t = 0.0;  // Current time (simplified)
        return S0 + (theta_(t) - a_ * S0) * dt + sigma_ * std::sqrt(dt) * random;
    }
    
    double getInitialPrice() const override { return r0_; }
    double getDrift() const override { return theta_(0.0); }
    double getVolatility() const override { return sigma_; }
    double getMeanReversion() const override { return a_; }
    
    void setThetaFunction(std::function<double(double)> func) {
        theta_ = func;
    }
    
    void setParameters(const std::vector<double>& params) override {
        if (params.size() >= 3) {
            r0_ = params[0];
            a_ = params[1];
            sigma_ = params[2];
        }
    }
    
    std::vector<double> getParameters() const override {
        return {r0_, a_, sigma_};
    }
};

#endif // HULLWHITE_H
