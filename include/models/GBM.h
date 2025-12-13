#ifndef GBM_H
#define GBM_H

#include "StochasticModel.h"
#include <string>
#include <vector>

class GBM : public StochasticModel {
private:
    double mu_;          // Drift (expected return)
    double sigma_;       // Volatility
    double S0_;          // Initial price
    
public:
    GBM(double initialPrice = 100.0, double drift = 0.05, 
         double volatility = 0.2);
    
    std::string getName() const override;
    double simulate(double S0, double dt, double random) const override;
    
    std::vector<double> simulatePath(
        double S0, 
        int steps, 
        double dt,
        const std::vector<double>& randoms
    ) const override;
    
    double getInitialPrice() const override;
    double getDrift() const override;
    double getVolatility() const override;
    
    void setParameters(const std::vector<double>& params) override;
    std::vector<double> getParameters() const override;
    
    // Additional methods
    bool validateParameters() const;
    double expectedValue(double t) const;
    double variance(double t) const;
};

#endif // GBM_H
