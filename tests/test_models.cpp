#include <gtest/gtest.h>
#include <memory>
#include <cmath>
#include <vector>

#include "models/GBM.h"
#include "models/Vasicek.h"
#include "models/HullWhite.h"

class GBMTest : public ::testing::Test {
protected:
    void SetUp() override {
        gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    }
    
    std::shared_ptr<GBM> gbm;
};

TEST_F(GBMTest, ConstructorAndGetters) {
    EXPECT_DOUBLE_EQ(gbm->getInitialPrice(), 100.0);
    EXPECT_DOUBLE_EQ(gbm->getDrift(), 0.05);
    EXPECT_DOUBLE_EQ(gbm->getVolatility(), 0.2);
    EXPECT_EQ(gbm->getName(), "Geometric Brownian Motion");
}

TEST_F(GBMTest, InvalidParameters) {
    EXPECT_THROW(GBM(-100.0, 0.05, 0.2), std::invalid_argument);
    EXPECT_THROW(GBM(100.0, 0.05, -0.2), std::invalid_argument);
}

TEST_F(GBMTest, ZeroVolatilitySimulation) {
    GBM zeroVol(100.0, 0.0, 0.0);
    double result = zeroVol.simulate(100.0, 1.0, 0.0);
    EXPECT_NEAR(result, 100.0, 1e-10);
}

TEST_F(GBMTest, PathSimulation) {
    std::vector<double> randoms = {0.1, -0.2, 0.3, -0.1, 0.0};
    auto path = gbm->simulatePath(100.0, 5, 1.0/252.0, randoms);
    
    EXPECT_EQ(path.size(), 5);
    EXPECT_GT(path[0], 0.0);
    
    // 验证路径中的每个值都是正数
    for (double price : path) {
        EXPECT_GT(price, 0.0);
    }
}

TEST_F(GBMTest, ExpectedValueAndVariance) {
    double t = 1.0;
    double expected = gbm->expectedValue(t);
    double variance = gbm->variance(t);
    
    EXPECT_GT(expected, 100.0);  // 正漂移率导致期望值增加
    EXPECT_GT(variance, 0.0);    // 正波动率导致方差为正
}

TEST_F(GBMTest, ParameterValidation) {
    EXPECT_TRUE(gbm->validateParameters());
    
    GBM invalidGBM(-100.0, 0.05, 0.2);
    EXPECT_FALSE(invalidGBM.validateParameters());
}

TEST_F(GBMTest, SetAndGetParameters) {
    std::vector<double> newParams = {150.0, 0.1, 0.3};
    gbm->setParameters(newParams);
    
    auto params = gbm->getParameters();
    EXPECT_EQ(params.size(), 3);
    EXPECT_DOUBLE_EQ(params[0], 150.0);
    EXPECT_DOUBLE_EQ(params[1], 0.1);
    EXPECT_DOUBLE_EQ(params[2], 0.3);
}

TEST_F(GBMTest, SimulationConsistency) {
    // 相同随机数应该产生相同结果
    double result1 = gbm->simulate(100.0, 0.1, 0.5);
    double result2 = gbm->simulate(100.0, 0.1, 0.5);
    
    EXPECT_DOUBLE_EQ(result1, result2);
}

class VasicekTest : public ::testing::Test {
protected:
    void SetUp() override {
        vasicek = std::make_shared<Vasicek>(0.05, 1.0, 0.05, 0.02);
    }
    
    std::shared_ptr<Vasicek> vasicek;
};

TEST_F(VasicekTest, ConstructorAndGetters) {
    EXPECT_DOUBLE_EQ(vasicek->getInitialPrice(), 0.05);
    EXPECT_DOUBLE_EQ(vasicek->getMeanReversion(), 1.0);
    EXPECT_DOUBLE_EQ(vasicek->getLongTermMean(), 0.05);
    EXPECT_DOUBLE_EQ(vasicek->getVolatility(), 0.02);
    EXPECT_EQ(vasicek->getName(), "Vasicek Model");
}

TEST_F(VasicekTest, Simulation) {
    // 测试模拟函数
    double result = vasicek->simulate(0.05, 0.1, 0.0);
    EXPECT_NEAR(result, 0.05, 0.001);  // 随机数为0时，结果应该接近当前值
}

TEST_F(VasicekTest, MeanReversionProperty) {
    // 测试均值回归特性
    double aboveMean = 0.10;  // 高于长期均值
    double result = vasicek->simulate(aboveMean, 0.1, 0.0);
    EXPECT_LT(result, aboveMean);  // 应该向均值回归
    
    double belowMean = 0.02;  // 低于长期均值
    result = vasicek->simulate(belowMean, 0.1, 0.0);
    EXPECT_GT(result, belowMean);  // 应该向均值回归
}

TEST_F(VasicekTest, ExpectedValue) {
    double t = 1.0;
    double expected = vasicek->expectedValue(t);
    
    // 对于均值回归模型，长期期望值应该接近长期均值
    EXPECT_NEAR(expected, 0.05, 0.01);
}

TEST_F(VasicekTest, Variance) {
    double t = 1.0;
    double variance = vasicek->variance(t);
    
    EXPECT_GT(variance, 0.0);  // 方差应该为正
    EXPECT_LT(variance, 1.0);  // 对于利率模型，方差不应该太大
}

class HullWhiteTest : public ::testing::Test {
protected:
    void SetUp() override {
        hullWhite = std::make_shared<HullWhite>(0.05, 0.1, 0.02);
    }
    
    std::shared_ptr<HullWhite> hullWhite;
};

TEST_F(HullWhiteTest, ConstructorAndGetters) {
    EXPECT_DOUBLE_EQ(hullWhite->getInitialPrice(), 0.05);
    EXPECT_DOUBLE_EQ(hullWhite->getMeanReversion(), 0.1);
    EXPECT_DOUBLE_EQ(hullWhite->getVolatility(), 0.02);
    EXPECT_EQ(hullWhite->getName(), "Hull-White Model");
}

TEST_F(HullWhiteTest, Simulation) {
    double result = hullWhite->simulate(0.05, 0.1, 0.0);
    // Hull-White模型有theta函数，结果取决于具体实现
    EXPECT_TRUE(std::isfinite(result));
}

TEST_F(HullWhiteTest, SetThetaFunction) {
    // 测试设置theta函数
    auto thetaFunc = [](double t) { return 0.03 + 0.01 * t; };
    
    EXPECT_NO_THROW(hullWhite->setThetaFunction(thetaFunc));
}

TEST_F(HullWhiteTest, ExpectedValue) {
    double t = 1.0;
    double expected = hullWhite->expectedValue(t);
    
    EXPECT_TRUE(std::isfinite(expected));
}

TEST_F(HullWhiteTest, ModelComparison) {
    // 比较不同模型的特性
    auto gbm = std::make_shared<GBM>(100.0, 0.05, 0.2);
    auto vasicek = std::make_shared<Vasicek>(0.05, 1.0, 0.05, 0.02);
    
    EXPECT_NE(gbm->getName(), vasicek->getName());
    EXPECT_NE(gbm->getVolatility(), vasicek->getVolatility());
}

TEST_F(HullWhiteTest, ParameterValidation) {
    EXPECT_TRUE(hullWhite->validateParameters());
    
    // 测试无效参数
    EXPECT_THROW(std::make_shared<HullWhite>(0.05, -0.1, 0.02), std::invalid_argument);
    EXPECT_THROW(std::make_shared<HullWhite>(0.05, 0.1, -0.02), std::invalid_argument);
}

// 测试随机数生成对模型的影响
TEST(ModelRandomnessTest, GBMWithDifferentRandoms) {
    GBM gbm(100.0, 0.05, 0.2);
    
    double result1 = gbm.simulate(100.0, 0.1, 1.0);   // 正随机数
    double result2 = gbm.simulate(100.0, 0.1, -1.0);  // 负随机数
    
    EXPECT_NE(result1, result2);
    EXPECT_GT(result1, result2);  // 正随机数应该产生更高价格
}

// 测试时间步长的影响
TEST(ModelTimeStepTest, TimeStepEffect) {
    GBM gbm(100.0, 0.05, 0.2);
    
    double smallStep = gbm.simulate(100.0, 0.01, 0.5);
    double largeStep = gbm.simulate(100.0, 0.1, 0.5);
    
    EXPECT_NE(smallStep, largeStep);
}

// 测试模型参数边界条件
TEST(ModelBoundaryTest, ExtremeParameters) {
    // 测试极端但有效的参数
    EXPECT_NO_THROW(GBM(1e-6, 0.0, 0.01));    // 极小初始价格
    EXPECT_NO_THROW(GBM(1e6, 0.5, 0.5));      // 大初始价格和高参数
    
    // 测试边界情况下的模拟
    GBM lowVol(100.0, 0.05, 0.001);
    double result = lowVol.simulate(100.0, 1.0, 0.0);
    EXPECT_NEAR(result, 100.0 * exp(0.05), 0.01);
}

// 测试模型序列化/反序列化
TEST(ModelSerializationTest, ParameterRoundTrip) {
    GBM original(100.0, 0.05, 0.2);
    auto params = original.getParameters();
    
    GBM restored(1.0, 0.0, 0.0);  // 初始化为不同值
    restored.setParameters(params);
    
    EXPECT_DOUBLE_EQ(original.getInitialPrice(), restored.getInitialPrice());
    EXPECT_DOUBLE_EQ(original.getDrift(), restored.getDrift());
    EXPECT_DOUBLE_EQ(original.getVolatility(), restored.getVolatility());
}

// 测试模型在不同场景下的行为
TEST(ModelScenarioTest, MarketScenarios) {
    // 牛市场景：正漂移
    GBM bullMarket(100.0, 0.10, 0.15);
    
    // 熊市场景：负漂移
    GBM bearMarket(100.0, -0.05, 0.20);
    
    // 高波动场景
    GBM highVol(100.0, 0.03, 0.40);
    
    // 模拟相同随机数下的结果
    double bullResult = bullMarket.simulate(100.0, 1.0, 0.0);
    double bearResult = bearMarket.simulate(100.0, 1.0, 0.0);
    double highVolResult = highVol.simulate(100.0, 1.0, 0.0);
    
    EXPECT_GT(bullResult, bearResult);  // 牛市应该表现更好
    // 高波动率模型可能会有极端结果
    EXPECT_TRUE(std::isfinite(highVolResult));
}
