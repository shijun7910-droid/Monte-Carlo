#ifndef VASICEK_H
#define VASICEK_H

#include "StochasticModel.h"
#include <string>
#include <vector>

class Vasicek : public StochasticModel {
private:
    double kappa_;       // Speed of mean reversion
    double theta_;       // Long-term mean
    double sigma_;       // Volatility
    double r0_;          // Initial interest rate
    
public:
    // Constructor
    Vasicek(double initialRate = 0.05, 
            double speedOfReversion = 1.0, 
            double longTermMean = 0.05, 
            double volatility = 0.02);
    
    // Get model name
    std::string getName() const override;
    
    // Single step simulation
    // dr(t) = kappa * (theta - r(t)) * dt + sigma * dW(t)
    double simulate(double currentRate, double dt, double random) const override;
    
    // Accessors
    double getInitialRate() const;        // Specific to interest rate models
    double getSpeedOfReversion() const;   // kappa
    double getLongTermMean() const;       // theta
    double getVolatility() const;         // sigma
    
    // Model-specific methods
    double analyticalMean(double t) const;
    double analyticalVariance(double t) const;
    double stationaryDistributionMean() const;
    double stationaryDistributionVariance() const;
    
    // Check if parameters are valid
    bool isParametersValid() const;
    
    // Generate full path
    std::vector<double> generatePath(
        int steps,
        double dt,
        const std::vector<double>& randoms
    ) const;
    
    // Check if model can produce negative rates
    bool canProduceNegativeRates() const;
    
    // Probability calculations
    double probabilityRateBelow(double threshold, double time) const;
    double probabilityRateAbove(double threshold, double time) const;
    
private:
    // Helper for analytical solutions
    double meanReversionFactor(double dt) const;
};

#endif // VASICEK_H
