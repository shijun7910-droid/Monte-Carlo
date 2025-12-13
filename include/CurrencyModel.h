#ifndef CURRENCYMODEL_H
#define CURRENCYMODEL_H

#include <vector>

class CurrencyModel {
public:
    virtual ~CurrencyModel() = default;
    virtual std::vector<double> generatePath(double initialRate, 
                                             double timeHorizon,
                                             int steps,
                                             const std::vector<double>& randomNumbers) = 0;
};

class GBM : public CurrencyModel {
private:
    double mu;    // Annual return rate
    double sigma; // Annual volatility
    
public:
    GBM(double mu, double sigma);
    std::vector<double> generatePath(double initialRate, 
                                     double timeHorizon,
                                     int steps,
                                     const std::vector<double>& randomNumbers) override;
};

class Vasicek : public CurrencyModel {
private:
    double kappa;  // Mean reversion speed
    double theta;  // Long-term mean
    double sigma;  // Volatility
    
public:
    Vasicek(double kappa, double theta, double sigma);
    std::vector<double> generatePath(double initialRate,
                                     double timeHorizon,
                                     int steps,
                                     const std::vector<double>& randomNumbers) override;
};

#endif // CURRENCYMODEL_H
