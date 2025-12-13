#ifndef VASICEK_H
#define VASICEK_H

#include "StochasticModel.h"
#include <string>
#include <vector>

class Vasicek : public StochasticModel {
private:
    double theta_;       // Speed of mean reversion
    double mu_;          // Long-term mean
    double sigma_;       // Volatility
    double r0_;          // Initial interest rate
    
public:
    Vasicek(double initialRate = 0.05, double speed = 1.0, 
            double longTermMean = 0.05, double volatility = 0.02)
        : r0_(initialRate), theta_(speed), mu_(longTermMean), sigma_(volatility) {}
    
    std::string getName() const override {
        return "Vasicek Model";
    }
    
    double simulate(double S0, double dt, double random) const override {
        return S0 + theta_ * (mu_ - S0) * dt + sigma_ * std::sqrt(dt) * random;
    }
    
    double getInitialPrice() const override { return r0_; }
    double getDrift() const override { return mu_; }
    double getVolatility() const override { return sigma_; }
    double getMeanReversion() const override { return theta_; }
    double getLongTermMean() const override { return mu_; }
    
    void setParameters(const std::vector<double>& params) override {
        if (params.size() >= 4) {
            r0_ = params[0];
            theta_ = params[1];
            mu_ = params[2];
            sigma_ = params[3];
        }
    }
    
    std::vector<double> getParameters() const override {
        return {r0_, theta_, mu_, sigma_};
    }
};

#endif // VASICEK_H
