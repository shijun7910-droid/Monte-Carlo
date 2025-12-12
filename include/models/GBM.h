#ifndef GBM_H
#define GBM_H

#include "StochasticModel.h"
#include <cmath>
#include <vector>

class GBM : public StochasticModel {
private:
    double mu_;          // 漂移率
    double sigma_;       // 波动率
    double S0_;          // 初始价格
    
public:
    GBM(double initialPrice = 100.0, double drift = 0.05, 
         double volatility = 0.2)
        : S0_(initialPrice), mu_(drift), sigma_(volatility) {}
    
    std::string getName() const override {
        return "Geometric Brownian Motion";
    }
    
    double simulate(double S0, double dt, double random) const override {
        return S0 * std::exp((mu_ - 0.5 * sigma_ * sigma_) * dt 
                           + sigma_ * std::sqrt(dt) * random);
    }
    
    std::vector<double> simulatePath(
        double S0, 
        int steps, 
        double dt,
        const std::vector<double>& randoms
    ) const override {
        std::vector<double> path(steps);
        double current = S0;
        
        for (int i = 0; i < steps; ++i) {
            current = simulate(current, dt, randoms[i]);
            path[i] = current;
        }
        
        return path;
    }
    
    double getDrift() const override { return mu_; }
    double getVolatility() const override { return sigma_; }
    double getInitialPrice() const override { return S0_; }
    
    void setParameters(double drift, double volatility) {
        mu_ = drift;
        sigma_ = volatility;
    }
    
    std::vector<double> calibrate(const std::vector<double>& historicalPrices) override {
        if (historicalPrices.size() < 2) return {mu_, sigma_};
        
        std::vector<double> returns;
        for (size_t i = 1; i < historicalPrices.size(); ++i) {
            double ret = std::log(historicalPrices[i] / historicalPrices[i-1]);
            returns.push_back(ret);
        }
        
        // 计算年化漂移率和波动率（假设252个交易日）
        double mean = 0.0;
        for (double ret : returns) mean += ret;
        mean /= returns.size();
        mu_ = mean * 252.0;  // 年化漂移率
        
        double variance = 0.0;
        for (double ret : returns) {
            variance += (ret - mean) * (ret - mean);
        }
        variance /= (returns.size() - 1);
        sigma_ = std::sqrt(variance * 252.0);  // 年化波动率
        
        return {mu_, sigma_};
    }
};

#endif // GBM_H
