#pragma once
#include <vector>
#include <cmath>

// 货币模型基类
class CurrencyModel {
public:
    virtual ~CurrencyModel() = default;
    virtual std::vector<double> generatePath(double initialRate, 
                                             double timeHorizon,
                                             int steps,
                                             const std::vector<double>& randomNumbers) = 0;
    virtual double drift(double rate, double time) = 0;
    virtual double diffusion(double rate, double time) = 0;
};

// 几何布朗运动模型 (GBM)
class GBM : public CurrencyModel {
private:
    double mu;    // 年化收益率
    double sigma; // 年化波动率
    
public:
    GBM(double mu, double sigma);
    std::vector<double> generatePath(double initialRate, 
                                     double timeHorizon,
                                     int steps,
                                     const std::vector<double>& randomNumbers) override;
    double drift(double rate, double time) override;
    double diffusion(double rate, double time) override;
};

// 均值回归模型 (Vasicek)
class Vasicek : public CurrencyModel {
private:
    double kappa;  // 回归速度
    double theta;  // 长期均值
    double sigma;  // 波动率
    
public:
    Vasicek(double kappa, double theta, double sigma);
    std::vector<double> generatePath(double initialRate,
                                     double timeHorizon,
                                     int steps,
                                     const std::vector<double>& randomNumbers) override;
    double drift(double rate, double time) override;
    double diffusion(double rate, double time) override;
};
