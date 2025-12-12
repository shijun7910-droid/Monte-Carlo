#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include "models/GBM.h"
#include "random/RandomGenerator.h"
#include "statistics/Statistics.h"

TEST(GBMTest, Initialization) {
    GBM gbm(100.0, 0.05, 0.2);
    EXPECT_DOUBLE_EQ(gbm.getInitialPrice(), 100.0);
    EXPECT_DOUBLE_EQ(gbm.getDrift(), 0.05);
    EXPECT_DOUBLE_EQ(gbm.getVolatility(), 0.2);
}

TEST(GBMTest, Simulation) {
    GBM gbm(100.0, 0.0, 0.0);  // 无漂移无波动
    double result = gbm.simulate(100.0, 1.0, 0.0);
    EXPECT_DOUBLE_EQ(result, 100.0);
}

TEST(GBMTest, PathSimulation) {
    GBM gbm(100.0, 0.05, 0.2);
    std::vector<double> randoms = {0.1, -0.2, 0.3, -0.1, 0.0};
    auto path = gbm.simulatePath(100.0, 5, 1.0/252, randoms);
    
    EXPECT_EQ(path.size(), 5);
    EXPECT_GT(path[0], 0.0);
}

TEST(StatisticsTest, BasicStatistics) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    EXPECT_DOUBLE_EQ(Statistics::mean(data), 3.0);
    EXPECT_DOUBLE_EQ(Statistics::median(data), 3.0);
    EXPECT_NEAR(Statistics::standardDeviation(data), 1.5811, 0.0001);
    EXPECT_NEAR(Statistics::variance(data), 2.5, 0.0001);
}

TEST(StatisticsTest, VaRCalculation) {
    std::vector<double> data = {10.0, 20.0, 30.0, 40.0, 50.0};
    double var95 = Statistics::valueAtRisk(data, 0.95);
    double var99 = Statistics::valueAtRisk(data, 0.99);
    
    EXPECT_DOUBLE_EQ(var95, 10.0);
    EXPECT_DOUBLE_EQ(var99, 10.0);
}

TEST(RandomGeneratorTest, NormalDistribution) {
    NormalGenerator gen(0.0, 1.0, 42);
    std::vector<double> values = gen.generateVector(1000);
    
    double mean = Statistics::mean(values);
    double stddev = Statistics::standardDeviation(values);
    
    EXPECT_NEAR(mean, 0.0, 0.1);
    EXPECT_NEAR(stddev, 1.0, 0.1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
