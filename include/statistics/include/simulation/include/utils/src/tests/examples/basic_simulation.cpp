#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

#include "models/GBM.h"
#include "random/RandomGenerator.h"
#include "simulation/MonteCarloSimulator.h"
#include "statistics/Statistics.h"
#include "utils/CSVWriter.h"

int main() {
    std::cout << "=== Basic Currency Simulation Example ===\n\n";
    
    // 模拟USD/RUB汇率
    double initialPrice = 75.0;    // 初始汇率
    double drift = 0.05;          // 5%年化漂移
    double volatility = 0.2;      // 20%年化波动率
    int numSimulations = 10000;   // 10,000次模拟
    int numSteps = 252;           // 252个交易日（1年）
    double dt = 1.0 / 252.0;      // 每日时间步长
    
    std::cout << "Parameters:\n";
    std::cout << "Initial USD/RUB rate: " << initialPrice << "\n";
    std::cout << "Annual drift (μ): " << drift << " (" << (drift * 100) << "%)\n";
    std::cout << "Annual volatility (σ): " << volatility << " (" << (volatility * 100) << "%)\n";
    std::cout << "Number of simulations: " << numSimulations << "\n";
    std::cout << "Time horizon: " << numSteps << " days (" << (numSteps/252.0) << " years)\n";
    std::cout << std::endl;
    
    // 创建模型
    auto gbm = std::make_shared<GBM>(initialPrice, drift, volatility);
    auto randomGen = std::make_shared<NormalGenerator>(0.0, 1.0, 12345);
    MonteCarloSimulator simulator(gbm, randomGen);
    
    // 运行模拟
    auto start = std::chrono::high_resolution_clock::now();
    auto result = simulator.runSimulation(numSimulations, numSteps, dt);
    auto end = std::chrono::high_resolution_clock::now();
    
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    // 显示结果
    std::cout << "\n=== Simulation Results ===\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";
    std::cout << "Simulations per second: " << (numSimulations / elapsed) << "\n\n";
    
    Statistics::printSummary(result.summary, "USD/RUB Exchange Rate Forecast");
    
    // 额外分析
    std::cout << "\n=== Probability Analysis ===\n";
    
    // 计算上涨10%的概率
    double target10 = initialPrice * 1.10;
    double target20 = initialPrice * 1.20;
    double targetDown10 = initialPrice * 0.90;
    
    int count10 = 0, count20 = 0, countDown10 = 0;
    for (double price : result.finalPrices) {
        if (price >= target10) count10++;
        if (price >= target20) count20++;
        if (price <= targetDown10) countDown10++;
    }
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Probability of rate >= " << target10 << " (+10%): " 
              << (100.0 * count10 / numSimulations) << "%\n";
    std::cout << "Probability of rate >= " << target20 << " (+20%): " 
              << (100.0 * count20 / numSimulations) << "%\n";
    std::cout << "Probability of rate <= " << targetDown10 << " (-10%): " 
              << (100.0 * countDown10 / numSimulations) << "%\n";
    
    // 保存结果
    CSVWriter::writeSummary(result.finalPrices, "usd_rub_forecast_summary.csv");
    
    // 保存样本路径用于可视化
    std::vector<std::vector<double>> samplePaths;
    for (int i = 0; i < std::min(100, numSimulations); ++i) {
        samplePaths.push_back(result.paths[i]);
    }
    CSVWriter::writePaths(samplePaths, "usd_rub_sample_paths.csv");
    
    std::cout << "\nResults saved to CSV files for visualization.\n";
    std::cout << "Use Python scripts to visualize the results.\n";
    
    return 0;
}
