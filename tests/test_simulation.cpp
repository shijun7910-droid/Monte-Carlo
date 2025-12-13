#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <cmath>

#include "models/GBM.h"
#include "random/RandomGenerator.h"
#include "simulation/MonteCarloSimulator.h"
#include "simulation/PathGenerator.h"
#include "simulation/ResultAnalyzer.h"

class MonteCarloSimulatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_shared<GBM>(100.0, 0.05, 0.2);
        randomGen = std::make_shared<NormalGenerator>(0.0, 1.0, 42);
        simulator = std::make_shared<MonteCarloSimulator>(model, randomGen);
    }
    
    std::shared_ptr<GBM> model;
    std::shared_ptr<NormalGenerator> randomGen;
    std::shared_ptr<MonteCarloSimulator> simulator;
};

TEST_F(MonteCarloSimulatorTest, ConstructorAndSetup) {
    EXPECT_NO_THROW(MonteCarloSimulator simulator(model, randomGen));
    
    // 测试无效输入
    EXPECT_THROW(MonteCarloSimulator(nullptr, randomGen), std::invalid_argument);
    EXPECT_THROW(MonteCarloSimulator(model, nullptr), std::invalid_argument);
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
}

TEST_F(MonteCarloSimulatorTest, InvalidParameters) {
    // 测试无效参数
    EXPECT_THROW(simulator->runSimulation(0, 10, 0.1), std::invalid_argument);      // 零路径
    EXPECT_THROW(simulator->runSimulation(10, 0, 0.1), std::invalid_argument);      // 零步长
    EXPECT_THROW(simulator->runSimulation(10, 10, 0.0), std::invalid_argument);     // 零时间步
    EXPECT_THROW(simulator->runSimulation(10, 10, -0.1), std::invalid_argument);    // 负时间步
    
    EXPECT_THROW(simulator->runSimulationBatch(10, 10, 0.1, 0), std::invalid_argument);  // 零批次大小
    EXPECT_THROW(simulator->runSimulationBatch(10, 10, 0.1, -1), std::invalid_argument); // 负批次大小
}

TEST_F(MonteCarloSimulatorTest, ThreadConfiguration) {
    // 测试线程配置
    EXPECT_NO_THROW(simulator->setNumThreads(1));
    EXPECT_NO_THROW(simulator->setNumThreads(4));
    
    // 无效线程数
    EXPECT_THROW(simulator->setNumThreads(0), std::invalid_argument);
    EXPECT_THROW(simulator->setNumThreads(-1), std::invalid_argument);
}

TEST_F(MonteCarloSimulatorTest, RiskMetricsCalculation) {
   
