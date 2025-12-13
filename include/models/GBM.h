#ifndef GBM_H
#define GBM_H

#include "StochasticModel.h"
#include <string>

class GBM : public StochasticModel {
private:
    double mu_;          // Drift coefficient
    double sigma_;       // Volatility coefficient
    double S0_;          // Initial price
    
public:
    // Constructor
    GBM(double initialPrice = 100.0, double drift = 0.05, double volatility = 0.2);
    
    // Get model name
    std::string getName() const override;
    
    // Single step simulation
    // Formula: S(t+dt) = S(t) * exp((mu - 0.5*sigma^2)*dt + sigma*sqrt(dt)*random)
    double simulate(double currentPrice, double dt, double random) const override;
    
    // Optional: If you need to simulate full path
    std::vector<double> simulatePath(
        double initialPrice, 
        int steps, 
        double dt,
        const std::vector<double>& randoms
    ) const;
    
    // Accessors
    double getInitialPrice() const;
    double getDrift() const;
    double getVolatility() const;
    
    // Setters
    void setDrift(double drift);
    void setVolatility(double volatility);
    void setInitialPrice(double initialPrice);
    
    // Validation
    bool isValid() const;
    
    // Analytical calculations
    double expectedPrice(double time) const;  // E[S(t)] = S0 * exp(mu * t)
    double confidenceInterval(double time, double confidenceLevel = 0.95) const;
    
private:
    // Helper function for simulation
    double calculateStep(double currentPrice, double dt, double random) const;
};

#endif // GBM_H
