#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <memory>
#include <stdexcept>

#include "models/GBM.h"
#include "models/Vasicek.h"
#include "random/RandomGenerator.h"
#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"
#include "simulation/MonteCarloSimulator.h"

// Test GBM model
TEST(GBMTest, ConstructorAndGetters) {
    GBM gbm(100.0, 0.05, 0.2);
    EXPECT_DOUBLE_EQ(gbm.getInitialPrice(), 100.0);
    EXPECT_DOUBLE_EQ(gbm.getDrift(), 0.05);
    EXPECT_DOUBLE_EQ(gbm.getVolatility(), 0.2);
    EXPECT_EQ(gbm.getName(), "Geometric Brownian Motion");
}

TEST(GBMTest, InvalidParameters) {
    EXPECT_THROW(GBM(-100.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(GBM(100.0, 0.05, -0.2), std::invalid_argument);
}

TEST(GBMTest, Simulation) {
    GBM gbm(100.0, 0.0, 0.0);
    double result = gbm.simulate(100.0, 1.0, 0.0);
    EXPECT_NEAR(result, 100.0, 1e-10);
}

TEST(GBMTest, PathSimulation) {
    GBM gbm(100.0, 0.05, 0.2);
    std::vector<double> randoms = {0.1, -0.2, 0.3, -0.1, 0.0};
    auto path = gbm.simulatePath(100.0, 5, 1.0/252.0, randoms);
    
    EXPECT_EQ(path.size(), 5);
    EXPECT_GT(path[0], 0.0);
}

TEST(GBMTest, ExpectedValueAndVariance) {
    GBM gbm(100.0, 0.05, 0.2);
    double t = 1.0;
    double expected = gbm.expectedValue(t);
    double variance = gbm.variance(t);
    
    EXPECT_GT(expected, 100.0);  // With positive drift
    EXPECT_GT(variance, 0.0);    // With volatility
}

// Test Vasicek model
TEST(VasicekTest, ConstructorAndGetters) {
    Vasicek vasicek(0.05, 1.0, 0.05, 0.02);
    EXPECT_DOUBLE_EQ(vasicek.getInitialPrice(), 0.05);
    EXPECT_DOUBLE_EQ(vasicek.getMeanReversion(), 1.0);
    EXPECT_DOUBLE_EQ(vasicek.getLongTermMean(), 0.05);
    EXPECT_DOUBLE_EQ(vasicek.getVolatility(), 0.02);
    EXPECT_EQ(vasicek.getName(), "Vasicek Model");
}

// Test Statistics
TEST(StatisticsTest, BasicStatistics) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    EXPECT_DOUBLE_EQ(Statistics::mean(data), 3.0);
    EXPECT_DOUBLE_EQ(Statistics::median(data), 3.0);
    EXPECT_NEAR(Statistics::standardDeviation(data), 1.5811, 0.0001);
    EXPECT_NEAR(Statistics::variance(data), 2.5, 0.0001);
}

TEST(StatisticsTest, Quantiles) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    EXPECT_DOUBLE_EQ(Statistics::quantile(data, 0.25), 2.0);
    EXPECT_DOUBLE_EQ(Statistics::quantile(data, 0.5), 3.0);
    EXPECT_DOUBLE_EQ(Statistics::quantile(data, 0.75), 4.0);
}

TEST(StatisticsTest, StatisticalSummary) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    auto summary = Statistics::analyze(data);
    
    EXPECT_DOUBLE_EQ(summary.mean, 3.0);
    EXPECT_DOUBLE_EQ(summary.median, 3.0);
    EXPECT_NEAR(summary.stdDev, 1.5811, 0.0001);
    EXPECT_DOUBLE_EQ(summary.min, 1.0);
    EXPECT_DOUBLE_EQ(summary.max, 5.0);
}

// Test RiskMetrics
TEST(RiskMetricsTest, VaRCalculation) {
    std::vector<double> returns = {0.01, 0.02, -0.03, 0.04, -0.02, 0.01, -0.05, 0.03, -0.01, 0.02};
    
    double var95 = RiskMetrics::calculateVaR(returns, 0.95);
    double var99 = RiskMetrics::calculateVaR(returns, 0.99);
    
    EXPECT_LT(var95, 0.0);
    EXPECT_LT(var99, 0.0);
    EXPECT_GE(var99, var95);
}

TEST(RiskMetricsTest, CVaRCalculation) {
    std::vector<double> returns = {0.01, 0.02, -0.03, 0.04, -0.02, 0.01, -0.05, 0.03, -0.01, 0.02};
    
    double cvar95 = RiskMetrics::calculateCVaR(returns, 0.95);
    double cvar99 = RiskMetrics::calculateCVaR(returns, 0.99);
    
    EXPECT_LT(cvar95, 0.0);
    EXPECT_LT(cvar99, 0.0);
}

TEST(RiskMetricsTest, VolatilityCalculation) {
    std::vector<double> returns = {0.01, -0.02, 0.03, -0.01, 0.02};
    
    double volatility = RiskMetrics::calculateVolatility(returns);
    EXPECT_GT(volatility, 0.0);
}

TEST(RiskMetricsTest, SharpeRatio) {
    std::vector<double> returns = {0.01, 0.02, 0.03, 0.02, 0.01};
    
    double sharpe = RiskMetrics::calculateSharpeRatio(returns, 0.02);
    EXPECT_GT(sharpe, 0.0);
}

// Test Random Generator
TEST(RandomGeneratorTest, NormalGenerator) {
    NormalGenerator gen(0.0, 1.0, 42);
    std::vector<double> values = gen.generateVector(1000);
    
    double mean = Statistics::mean(values);
    double stddev = Statistics::standardDeviation(values);
    
    EXPECT_NEAR(mean, 0.0, 0.1);
    EXPECT_NEAR(stddev, 1.0, 0.1);
}

TEST(RandomGeneratorTest, SeedSetting) {
    NormalGenerator gen1(0.0, 1.0, 42);
    NormalGenerator gen2(0.0, 1.0, 42);
    
    std::vector<double> values1 = gen1.generateVector(100);
    std::vector<double> values2 = gen2.generateVector(100);
    
    // With same seed, should get same sequence
    EXPECT_EQ(values1, values2);
}

// Test MonteCarloSimulator
TEST(MonteCarloSimulatorTest, ConstructorAndSetters) {
    auto gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    auto simulator = std::make_shared<MonteCarloSimulator>(gbm);
    
    EXPECT_NO_THROW(simulator->setSeed(12345));
    EXPECT_NO_THROW(simulator->setNumThreads(2));
    
    auto vasicek = std::make_shared<Vasicek>(0.05, 1.0, 0.05, 0.02);
    EXPECT_NO_THROW(simulator->setModel(vasicek));
}

TEST(MonteCarloSimulatorTest, InvalidParameters) {
    auto gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    auto simulator = std::make_shared<MonteCarloSimulator>(gbm);
    
    EXPECT_THROW(simulator->setNumThreads(0), std::invalid_argument);
    EXPECT_THROW(simulator->setNumThreads(-1), std::invalid_argument);
}

TEST(MonteCarloSimulatorTest, SmallSimulation) {
    auto gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    auto simulator = std::make_shared<MonteCarloSimulator>(gbm);
    simulator->setSeed(42);
    
    auto result = simulator->runSimulation(100, 10, 0.1);
    
    EXPECT_EQ(result.paths.size(), 100);
    EXPECT_EQ(result.finalPrices.size(), 100);
    EXPECT_EQ(result.returns.size(), 100);
    EXPECT_GT(result.executionTime, 0.0);
}

TEST(MonteCarloSimulatorTest, PercentileCalculation) {
    auto gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    auto simulator = std::make_shared<MonteCarloSimulator>(gbm);
    
    std::vector<double> prices = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    std::vector<double> percentiles = {0.25, 0.5, 0.75};
    
    auto results = simulator->calculatePercentiles(prices, percentiles);
    
    EXPECT_EQ(results.size(), 3);
    EXPECT_DOUBLE_EQ(results[0], 3.25);  // 25th percentile
    EXPECT_DOUBLE_EQ(results[1], 5.5);   // 50th percentile (median)
    EXPECT_DOUBLE_EQ(results[2], 7.75);  // 75th percentile
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
