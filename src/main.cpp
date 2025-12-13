#include <iostream>
#include <iomanip>
#include <fstream>
#include "MonteCarlo.h"
#include "CurrencyModel.h"
#include "RandomGenerator.h"
#include "Statistics.h"

void runGBMSimulation() {
    std::cout << "=== GBM 模型模拟 USD/EUR 汇率 ===" << std::endl;
    
    // 参数设置
    double initialRate = 0.92;  // 初始汇率
    double mu = 0.02;           // 年化收益率 2%
    double sigma = 0.15;        // 年化波动率 15%
    int numSimulations = 10000;
    int timeSteps = 252;        // 一年交易日
    double timeHorizon = 1.0;   // 1年
    
    // 创建模型和随机数生成器
    auto model = std::make_unique<GBM>(mu, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(42);
    
    // 创建模拟器
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, timeSteps, timeHorizon);
    
    // 运行模拟
    auto results = simulator.runSimulation(initialRate);
    
    // 计算风险指标
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    // 输出结果
    std::cout << "\n模拟结果统计:" << std::endl;
    std::cout << "初始汇率: " << initialRate << std::endl;
    std::cout << "模拟次数: " << numSimulations << std::endl;
    std::cout << "时间步长: " << timeSteps << std::endl;
    std::cout << "时间范围: " << timeHorizon << " 年" << std::endl;
    std::cout << "\n最终汇率分布:" << std::endl;
    std::cout << "均值: " << metrics.mean << std::endl;
    std::cout << "标准差: " << metrics.standardDeviation << std::endl;
    std::cout << "最小值: " << metrics.minValue << std::endl;
    std::cout << "最大值: " << metrics.maxValue << std::endl;
    std::cout << "中位数: " << metrics.median << std::endl;
    std::cout << "95% VaR: " << metrics.var95 << std::endl;
    std::cout << "95% CVaR: " << metrics.cvar95 << std::endl;
    
    // 保存部分路径到CSV
    std::ofstream file("gbm_paths.csv");
    if (file.is_open()) {
        // 写入时间点
        file << "Time";
        for (int i = 0; i < 10 && i < results.paths.size(); i++) {
            file << ",Path_" << i;
        }
        file << "\n";
        
        // 写入数据
        for (int t = 0; t < timeSteps; t++) {
            file << results.timePoints[t];
            for (int i = 0; i < 10 && i < results.paths.size(); i++) {
                file << "," << results.paths[i][t];
            }
            file << "\n";
        }
        file.close();
        std::cout << "\n路径数据已保存到 gbm_paths.csv" << std::endl;
    }
}

void runVasicekSimulation() {
    std::cout << "\n=== Vasicek 模型模拟利率 ===" << std::endl;
    
    // 参数设置
    double initialRate = 0.05;  // 初始利率 5%
    double kappa = 0.1;         // 回归速度
    double theta = 0.05;        // 长期均值
    double sigma = 0.02;        // 波动率
    int numSimulations = 5000;
    
    auto model = std::make_unique<Vasicek>(kappa, theta, sigma);
    auto randomGen = std::make_unique<MersenneTwister>(123);
    
    MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                                  numSimulations, 252, 1.0);
    
    auto results = simulator.runSimulation(initialRate);
    auto metrics = simulator.calculateRiskMetrics(results.finalValues);
    
    std::cout << "\nVasicek 模型结果:" << std::endl;
    std::cout << "均值: " << metrics.mean << " (长期均值: " << theta << ")" << std::endl;
    std::cout << "标准差: " << metrics.standardDeviation << std::endl;
}

int main() {
    std::cout << "货币汇率蒙特卡洛模拟系统\n" << std::endl;
    
    try {
        runGBMSimulation();
        runVasicekSimulation();
        
        std::cout << "\n模拟完成！" << std::endl;
        std::cout << "运行 'python scripts/plot_results.py' 查看可视化结果" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
