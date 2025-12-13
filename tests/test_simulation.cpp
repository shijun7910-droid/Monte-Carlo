#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <cmath>

#include "models/GBM.h"
#include "models/Vasicek.h"
#include "random/RandomGenerator.h"
#include "random/SobolGenerator.h"
#include "simulation/MonteCarloSimulator.h"
#include "simulation/PathGenerator.h"
#include "simulation/ResultAnalyzer.h"
#include "statistics/RiskMetrics.h"

class MonteCarloSimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        gbmModel = std::make_shared<GBM>(100.0, 0.05, 0.2);
        vasicekModel = std::make_shared<Vasicek>(0.05, 0.1, 0.02, 0.01);
        randomGen = std::make_shared<NormalGenerator>(0.0, 1.0, 42);
        sobolGen = std::make_shared<SobolGenerator>();
        simulator = std::make_shared<MonteCarloSimulator>(gbmModel, randomGen);
    }
    
    std::shared_ptr<GBM> gbmModel;
    std::shared_ptr<Vasicek> vasicekModel;
    std::shared_ptr<NormalGenerator> randomGen;
    std::shared_ptr<SobolGenerator> sobolGen;
    std::shared_ptr<MonteCarloSimulator> simulator;
};

TEST_F(MonteCarloSimulatorTest, ConstructorAndSetup) {
    EXPECT_NO_THROW(MonteCarloSimulator simulator(gbmModel, randomGen));
    
    // 测试无效输入
    EXPECT_THROW(MonteCarloSimulator(nullptr, randomGen), std::invalid_argument);
    EXPECT_THROW(MonteCarloSimulator(gbmModel, nullptr), std::invalid_argument);
    
    // 测试不同类型的随机数生成器
    EXPECT_NO_THROW(MonteCarloSimulator simulator(gbmModel, sobolGen));
}

TEST_F(MonteCarloSimulatorTest, BasicSimulation) {
    int numPaths = 100;
    int steps = 10;
    double dt = 0.1;
    
    auto result = simulator->runSimulation(numPaths, steps, dt);
    
    EXPECT_EQ(result.paths.size(), numPaths);
    EXPECT_EQ(result.finalPrices.size(), numPaths);
    EXPECT_EQ(result.returns.size(), numPaths);
    EXPECT_GT(result.executionTime, 0.0);
    
    // 验证每条路径的长度
    for (const auto& path : result.paths) {
        EXPECT_EQ(path.size(), steps);
    }
    
    // 验证初始价格正确
    for (const auto& path : result.paths) {
        EXPECT_NEAR(path[0], gbmModel->getInitialPrice(), 1e-10);
    }
}

TEST_F(MonteCarloSimulatorTest, SimulationConsistency) {
    // 相同种子应该产生相同结果
    simulator->setSeed(12345);
    auto result1 = simulator->runSimulation(50, 5, 0.1);
    
    simulator->setSeed(12345);
    auto result2 = simulator->runSimulation(50, 5, 0.1);
    
    // 最终价格应该相同
    for (size_t i = 0; i < result1.finalPrices.size(); ++i) {
        EXPECT_DOUBLE_EQ(result1.finalPrices[i], result2.finalPrices[i]);
    }
}

TEST_F(MonteCarloSimulatorTest, BatchSimulation) {
    int numPaths = 10000;
    int steps = 252;
    double dt = 1.0 / 252.0;
    int batchSize = 1000;
    
    auto result = simulator->runSimulationBatch(numPaths, steps, dt, batchSize);
    
    EXPECT_EQ(result.finalPrices.size(), numPaths);
    EXPECT_EQ(result.returns.size(), numPaths);
    EXPECT_GT(result.executionTime, 0.0);
    
    // 验证统计摘要被计算
    EXPECT_NE(result.priceSummary.mean, 0.0);
    EXPECT_NE(result.returnSummary.mean, 0.0);
    
    // 验证标准差为正数
    EXPECT_GT(result.priceSummary.stdDev, 0.0);
    EXPECT_GT(result.returnSummary.stdDev, 0.0);
    
    // 验证批次大小正确
    EXPECT_EQ(result.batchSize, batchSize);
}

TEST_F(MonteCarloSimulatorTest, InvalidParameters) {
    // 测试无效参数
    EXPECT_THROW(simulator->runSimulation(0, 10, 0.1), std::invalid_argument);      // 零路径
    EXPECT_THROW(simulator->runSimulation(10, 0, 0.1), std::invalid_argument);      // 零步长
    EXPECT_THROW(simulator->runSimulation(10, 10, 0.0), std::invalid_argument);     // 零时间步
    EXPECT_THROW(simulator->runSimulation(10, 10, -0.1), std::invalid_argument);    // 负时间步
    
    EXPECT_THROW(simulator->runSimulationBatch(10, 10, 0.1, 0), std::invalid_argument);  // 零批次大小
    EXPECT_THROW(simulator->runSimulationBatch(10, 10, 0.1, -1), std::invalid_argument); // 负批次大小
    EXPECT_THROW(simulator->runSimulationBatch(10, 10, 0.1, 20), std::invalid_argument); // 批次大小大于总路径数
}

TEST_F(MonteCarloSimulatorTest, ThreadConfiguration) {
    // 测试线程配置
    EXPECT_NO_THROW(simulator->setNumThreads(1));
    EXPECT_NO_THROW(simulator->setNumThreads(4));
    
    // 测试不同线程数的模拟结果应该相似（统计意义上）
    std::vector<double> means;
    for (int threads : {1, 2, 4}) {
        simulator->setNumThreads(threads);
        auto result = simulator->runSimulation(1000, 100, 0.01);
        means.push_back(result.priceSummary.mean);
    }
    
    // 均值应该接近（允许有小的差异）
    for (size_t i = 1; i < means.size(); ++i) {
        EXPECT_NEAR(means[i], means[0], std::abs(means[0] * 0.1)); // 允许10%的差异
    }
    
    // 无效线程数
    EXPECT_THROW(simulator->setNumThreads(0), std::invalid_argument);
    EXPECT_THROW(simulator->setNumThreads(-1), std::invalid_argument);
}

TEST_F(MonteCarloSimulatorTest, RiskMetricsCalculation) {
    int numPaths = 10000;
    int steps = 252;
    double dt = 1.0 / 252.0;
    
    auto result = simulator->runSimulation(numPaths, steps, dt);
    
    // 计算风险度量
    RiskMetrics riskMetrics;
    double var95 = riskMetrics.calculateVaR(result.finalPrices, 0.95);
    double var99 = riskMetrics.calculateVaR(result.finalPrices, 0.99);
    double cvar95 = riskMetrics.calculateCVaR(result.finalPrices, 0.95);
    double cvar99 = riskMetrics.calculateCVaR(result.finalPrices, 0.99);
    
    // VaR和CVaR应该为负数（因为这是损失）
    EXPECT_LT(var95, 0.0);
    EXPECT_LT(var99, 0.0);
    EXPECT_LT(cvar95, 0.0);
    EXPECT_LT(cvar99, 0.0);
    
    // 更高置信度的VaR应该更小（更大损失）
    EXPECT_LT(var99, var95);
    EXPECT_LT(cvar99, cvar95);
    
    // CVaR应该比对应VaR更小（更大损失）
    EXPECT_LT(cvar95, var95);
    EXPECT_LT(cvar99, var99);
}

TEST_F(MonteCarloSimulatorTest, DifferentModels) {
    // 测试不同模型
    int numPaths = 1000;
    int steps = 100;
    double dt = 0.01;
    
    // 测试GBM模型
    auto simulatorGBM = std::make_shared<MonteCarloSimulator>(gbmModel, randomGen);
    auto resultGBM = simulatorGBM->runSimulation(numPaths, steps, dt);
    
    // 测试Vasicek模型
    auto simulatorVasicek = std::make_shared<MonteCarloSimulator>(vasicekModel, randomGen);
    auto resultVasicek = simulatorVasicek->runSimulation(numPaths, steps, dt);
    
    // 两个模型的结果应该不同
    EXPECT_NE(resultGBM.priceSummary.mean, resultVasicek.priceSummary.mean);
    
    // 验证Vasicek模型的均值回归特性
    double vasicekMean = vasicekModel->getLongTermMean();
    EXPECT_NEAR(resultVasicek.priceSummary.mean, vasicekMean, 0.1);
}

TEST_F(MonteCarloSimulatorTest, MemoryManagement) {
    // 测试内存管理
    int numPaths = 10000;
    int steps = 1000;
    double dt = 0.001;
    
    // 测试是否正确处理大量数据
    EXPECT_NO_THROW({
        auto result = simulator->runSimulation(numPaths, steps, dt);
        // 验证内存使用
        EXPECT_EQ(result.paths.size(), numPaths);
        EXPECT_EQ(result.paths[0].size(), steps);
    });
    
    // 测试批次模拟的内存管理
    EXPECT_NO_THROW({
        auto result = simulator->runSimulationBatch(numPaths, steps, dt, 1000);
        EXPECT_EQ(result.finalPrices.size(), numPaths);
    });
}

TEST_F(MonteCarloSimulatorTest, ConvergenceTest) {
    // 测试收敛性：更多路径应该得到更稳定的结果
    std::vector<int> pathCounts = {100, 1000, 10000};
    std::vector<double> stdDevs;
    
    for (int numPaths : pathCounts) {
        auto result = simulator->runSimulation(numPaths, 100, 0.01);
        stdDevs.push_back(result.priceSummary.stdErr);
    }
    
    // 随着路径数增加，标准误应该减小
    for (size_t i = 1; i < stdDevs.size(); ++i) {
        EXPECT_LT(stdDevs[i], stdDevs[i-1] * 1.5); // 应该显著减小
    }
}

TEST_F(MonteCarloSimulatorTest, PathGeneratorIntegration) {
    // 测试PathGenerator的集成
    PathGenerator pathGenerator(gbmModel, randomGen);
    
    int numPaths = 100;
    int steps = 10;
    double dt = 0.1;
    
    auto paths = pathGenerator.generatePaths(numPaths, steps, dt);
    
    EXPECT_EQ(paths.size(), numPaths);
    for (const auto& path : paths) {
        EXPECT_EQ(path.size(), steps);
    }
    
    // 验证可以使用PathGenerator的结果
    EXPECT_NO_THROW({
        ResultAnalyzer analyzer;
        auto analysis = analyzer.analyzePaths(paths);
        EXPECT_GT(analysis.mean, 0.0);
    });
}

TEST_F(MonteCarloSimulatorTest, RandomGeneratorSeeding) {
    // 测试随机数生成器的种子设置
    auto randomGen1 = std::make_shared<NormalGenerator>(0.0, 1.0, 123);
    auto randomGen2 = std::make_shared<NormalGenerator>(0.0, 1.0, 123);
    auto randomGen3 = std::make_shared<NormalGenerator>(0.0, 1.0, 456);
    
    auto simulator1 = std::make_shared<MonteCarloSimulator>(gbmModel, randomGen1);
    auto simulator2 = std::make_shared<MonteCarloSimulator>(gbmModel, randomGen2);
    auto simulator3 = std::make_shared<MonteCarloSimulator>(gbmModel, randomGen3);
    
    auto result1 = simulator1->runSimulation(100, 10, 0.1);
    auto result2 = simulator2->runSimulation(100, 10, 0.1);
    auto result3 = simulator3->runSimulation(100, 10, 0.1);
    
    // 相同种子应该产生相同结果
    for (size_t i = 0; i < result1.finalPrices.size(); ++i) {
        EXPECT_DOUBLE_EQ(result1.finalPrices[i], result2.finalPrices[i]);
    }
    
    // 不同种子应该产生不同结果
    bool allEqual = true;
    for (size_t i = 0; i < result1.finalPrices.size(); ++i) {
        if (result1.finalPrices[i] != result3.finalPrices[i]) {
            allEqual = false;
            break;
        }
    }
    EXPECT_FALSE(allEqual);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
