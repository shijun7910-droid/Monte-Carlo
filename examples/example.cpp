#include <iostream>
#include <iomanip>
#include "MonteCarlo.h"
#include "CurrencyModel.h"
#include "RandomGenerator.h"

int main() {
    std::cout << "蒙特卡洛模拟示例程序\n" << std::endl;
    
    // 示例1: 使用GBM模型
    {
        std::cout << "示例1: GBM模型模拟USD/EUR汇率" << std::endl;
        auto model = std::make_unique<GBM>(0.03, 0.2);  // 3%收益, 20%波动
        auto randomGen = std::make_unique<MersenneTwister>(1234);
        
        MonteCarloSimulator simulator(std::move(model), std::move(randomGen), 
                                      1000, 100, 0.5);  // 半年
        
        auto results = simulator.runSimulation(1.0);
        auto metrics = simulator.calculateRiskMetrics(results.finalValues);
        
        std::cout << "6个月后汇率预测:" << std::endl;
        std::cout << "均值: " << std::fixed << std::setprecision(4) << metrics.mean << std::endl;
        std::cout << "95% VaR: " << metrics.var95 << std::endl;
        std::cout << std::endl;
    }
    
    // 示例2: 使用Vasicek模型
    {
        std::cout << "示例2: Vasicek模型模拟利率" << std::endl;
        auto model = std::make_unique<Vasicek>(0.5, 0.04, 0.02);  // 快速回归到4%
        auto randomGen = std::make_unique<SobolGenerator>();
        
        MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                      2000, 50, 1.0);  // 1年
        
        auto results = simulator.runSimulation(0.06);  // 初始利率6%
        auto metrics = simulator.calculateRiskMetrics(results.finalValues);
        
        std::cout << "1年后利率预测:" << std::endl;
        std::cout << "均值: " << std::fixed << std::setprecision(4) << metrics.mean << std::endl;
        std::cout << "标准差: " << metrics.standardDeviation << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
