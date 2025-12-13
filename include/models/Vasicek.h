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
            double longTermMean = 0.05, double volatility = 0.02);
    
    std::string getName() const override;
    double simulate(double S0, double dt, double random) const override;
    
    double getInitialPrice() const override;
    double getDrift() const override;
    double getVolatility() const override;
    double getMeanReversion() const override;
    double getLongTermMean() const override;
    
    void setParameters(const std::vector<double>& params) override;
    std::vector<double> getParameters() const override;
    
    // Additional methods
    bool validateParameters() const;
    double expectedValue(double t) const;
    double variance(double t) const;
};

#endif // VASICEK_H
