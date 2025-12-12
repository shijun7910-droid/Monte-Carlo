#ifndef STOCHASTIC_MODEL_H
#define STOCHASTIC_MODEL_H

#include <vector>
#include <memory>
#include <string>

class StochasticModel {
public:
    virtual ~StochasticModel() = default;
    
    virtual std::string getName() const = 0;
    virtual double simulate(double S0, double dt, double random) const = 0;
    virtual std::vector<double> simulatePath(
        double S0, 
        int steps, 
        double dt,
        const std::vector<double>& randoms
    ) const = 0;
    
    virtual double getDrift() const { return 0.0; }
    virtual double getVolatility() const { return 0.0; }
    virtual double getInitialPrice() const { return 0.0; }
    
    virtual std::vector<double> calibrate(const std::vector<double>& historicalPrices) {
        return {};
    }
};

#endif // STOCHASTIC_MODEL_H
