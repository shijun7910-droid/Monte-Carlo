#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <algorithm>

#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"
#include "statistics/Convergence.h"

class StatisticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        testData = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
        emptyData = {};
        singleData = {5.0};
        negativeData = {-3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0};
    }
    
    std::vector<double> testData;
    std::vector<double> emptyData;
    std::vector<double> singleData;
    std::vector<double> negativeData;
};

TEST_F(StatisticsTest, MeanCalculation) {
    double mean = Statistics::mean(testData);
    EXPECT_DOUBLE_EQ(mean, 5.5);
    
    // 空数据
    EXPECT_DOUBLE_EQ(Statistics::mean(emptyData), 0.0);
    
    // 单元素数据
    EXPECT_DOUBLE_EQ(Statistics::mean(singleData), 5.0);
}

TEST_F(StatisticsTest, MedianCalculation) {
    double median = Statistics::median(testData);
    EXPECT_DOUBLE_EQ(median, 5.5);
    
    // 奇数个元素
    std::vector<double> oddData = {1.0, 3.0, 2.0, 5.0, 4.0};
    double oddMedian = Statistics::median(oddData);
    EXPECT_DOUBLE_EQ(oddMedian, 3.0);
    
    // 空数据
    EXPECT_DOUBLE_EQ(Statistics::median(emptyData), 0.0);
}

TEST_F(StatisticsTest, StandardDeviation) {
    double stdDev = Statistics::standardDeviation(testData);
    EXPECT_NEAR(stdDev, 3.02765, 0.0001);
    
    // 单元素数据
    EXPECT_DOUBLE_EQ(Statistics::standardDeviation(singleData), 0.0);
    
    // 空数据
    EXPECT_DOUBLE_EQ(Statistics::standardDeviation(emptyData), 0.0);
}

TEST_F(StatisticsTest, Variance) {
    double variance = Statistics::variance(testData);
    EXPECT_NEAR(variance, 9.16667, 0.0001);
    
    // 验证方差与标准差的关系
    double stdDev = Statistics::standardDeviation(testData);
    EXPECT_NEAR(variance, stdDev * stdDev, 0.0001);
}

TEST_F(StatisticsTest, SkewnessAndKurtosis) {
    // 对称数据应该偏度接近0
    std::vector<double> symmetricData = {-2.0, -1.0, 0.0, 1.0, 2.0};
    double skewness = Statistics::skewness(symmetricData);
    EXPECT_NEAR(skewness, 0.0, 0.001);
    
    // 正偏数据
    std::vector<double> positiveSkewData = {1.0, 2.0, 3.0, 4.0, 10.0};
    double positiveSkew = Statistics::skewness(positiveSkewData);
    EXPECT_GT(positiveSkew, 0.0);
    
    // 峰度计算
    double kurtosis = Statistics::kurtosis(testData);
    EXPECT_TRUE(std::isfinite(kurtosis));
}

TEST_F(StatisticsTest, QuantileCalculation) {
    // 测试各种分位数
    EXPECT_DOUBLE_EQ(Statistics::quantile(testData, 0.0), 1.0);   // 最小值
    EXPECT_DOUBLE_EQ(Statistics::quantile(testData, 1.0), 10.0);  // 最大值
    EXPECT_DOUBLE_EQ(Statistics::quantile(testData, 0.5), 5.5);   // 中位数
    
    // 25%分位数
    double q25 = Statistics::quantile(testData, 0.25);
    EXPECT_NEAR(q25, 3.25, 0.001);
    
    // 75%分位数
    double q75 = Statistics::quantile(testData, 0.75);
    EXPECT_NEAR(q75, 7.75, 0.001);
    
    // 边界测试
    EXPECT_DOUBLE_EQ(Statistics::quantile(testData, -0.1), 1.0);   // 小于0
    EXPECT_DOUBLE_EQ(Statistics::quantile(testData, 1.1), 10.0);   // 大于1
}

TEST_F(StatisticsTest, ConfidenceInterval) {
    auto ci95 = Statistics::confidenceInterval(testData, 0.95);
    auto ci99 = Statistics::confidenceInterval(testData, 0.99);
    
    // 置信区间应该在均值附近
    double mean = Statistics::mean(testData);
    EXPECT_LT(ci95.first, mean);
    EXPECT_GT(ci95.second, mean);
    
    // 99%置信区间应该比95%更宽
    double width95 = ci95.second - ci95.first;
    double width99 = ci99.second - ci99.first;
    EXPECT_GT(width99, width95);
    
    // 无效置信水平
    auto invalidCI = Statistics::confidenceInterval(testData, 1.5);
    EXPECT_DOUBLE_EQ(invalidCI.first, 0.0);
    EXPECT_DOUBLE_EQ(invalidCI.second, 0.0);
}

TEST_F(StatisticsTest, StatisticalSummary) {
    auto summary = Statistics::analyze(testData);
    
    EXPECT_DOUBLE_EQ(summary.mean, 5.5);
    EXPECT_DOUBLE_EQ(summary.median, 5.5);
    EXPECT_DOUBLE_EQ(summary.min, 1.0);
    EXPECT_DOUBLE_EQ(summary.max, 10.0);
    EXPECT_DOUBLE_EQ(summary.quantile25, 3.25);
    EXPECT_DOUBLE_EQ(summary.quantile50, 5.5);
    EXPECT_DOUBLE_EQ(summary.quantile75, 7.75);
    
    // 验证置信区间计算正确
    EXPECT_LT(summary.confidenceInterval95.first, summary.mean);
    EXPECT_GT(summary.confidenceInterval95.second, summary.mean);
}

class RiskMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        returns = {0.01, 0.02, -0.03, 0.04, -0.02, 0.01, -0.05, 0.03, -0.01, 0.02};
        allPositiveReturns = {0.01, 0.02, 0.03, 0.02, 0.01};
        allNegativeReturns = {-0.01, -0.02, -0.03, -0.02, -0.01};
    }
    
    std::vector<double> returns;
    std::vector<double> allPositiveReturns;
    std::vector<double> allNegativeReturns;
};

TEST_F(RiskMetricsTest, ValueAtRiskCalculation) {
    double var90 = RiskMetrics::calculateVaR(returns, 0.90);
    double var95 = RiskMetrics::calculateVaR(returns, 0.95);
    double var99 = RiskMetrics::calculateVaR(returns, 0.99);
    
    // VaR应该是负数（表示损失）
    EXPECT_LT(var90, 0.0);
    EXPECT_LT(var95, 0.0);
    EXPECT_LT(var99, 0.0);
    
    // 更高置信水平的VaR应该更极端
    EXPECT_GE(var99, var95);
    EXPECT_GE(var95, var90);
    
    // 全是正收益的情况
    double positiveVar = RiskMetrics::calculateVaR(allPositiveReturns, 0.95);
    EXPECT_TRUE(positiveVar <= 0.0);  // 可能为0或负数
}

TEST_F(RiskMetricsTest, ConditionalVaRCalculation) {
    double cvar90 = RiskMetrics::calculateCVaR(returns, 0.90);
    double cvar95 = RiskMetrics::calculateCVaR(returns, 0.95);
    
    // CVaR应该是负数且比VaR更极端
    EXPECT_LT(cvar90, 0.0);
    EXPECT_LT(cvar95, 0.0);
    
    double var95 = RiskMetrics::calculateVaR(returns, 0.95);
    EXPECT_LE(cvar95, var95);  // CVaR应该 <= VaR
    
    // 全是负收益的情况
    double negativeCVaR = RiskMetrics::calculateCVaR(allNegativeReturns, 0.95);
    EXPECT_LT(negativeCVaR, 0.0);
}

TEST_F(RiskMetricsTest, VolatilityCalculation) {
    double volatility = RiskMetrics::calculateVolatility(returns);
    
    EXPECT_GT(volatility, 0.0);
    EXPECT_TRUE(std::isfinite(volatility));
    
    // 常数序列的波动率应为0
    std::vector<double> constantReturns(10, 0.01);
    double zeroVolatility = RiskMetrics::calculateVolatility(constantReturns);
    EXPECT_DOUBLE_EQ(zeroVolatility, 0.0);
}

TEST_F(RiskMetricsTest, SharpeRatio) {
    double sharpe = RiskMetrics::calculateSharpeRatio(returns, 0.03);
    
    EXPECT_TRUE(std::isfinite(sharpe));
    
    // 全是正收益应该有正夏普比率
    double positiveSharpe = RiskMetrics::calculateSharpeRatio(allPositiveReturns, 0.03);
    EXPECT_GT(positiveSharpe, 0.0);
    
    // 全是负收益可能有负夏普比率
    double negativeSharpe = RiskMetrics::calculateSharpeRatio(allNegativeReturns, 0.03);
    EXPECT_LT(negativeSharpe, 0.0);
    
    // 零波动率的情况
    std::vector<double> zeroVolReturns(10, 0.01);
    double zeroVolSharpe = RiskMetrics::calculateSharpeRatio(zeroVolReturns, 0.03);
    EXPECT_DOUBLE_EQ(zeroVolSharpe, 0.0);
}

TEST_F(RiskMetricsTest, MaximumDrawdown) {
    std::vector<double> prices = {100.0, 105.0, 95.0, 110.0, 90.0, 115.0, 85.0, 120.0, 80.0, 125.0};
    double drawdown = RiskMetrics::calculateMaxDrawdown(prices);
    
    EXPECT_GT(drawdown, 0.0);
    EXPECT_LT(drawdown, 1.0);  // 回撤应该在0-1之间
    
    // 单调上涨的情况
    std::vector<double> increasingPrices = {100.0, 101.0, 102.0, 103.0, 104.0};
    double zeroDrawdown = RiskMetrics::calculateMaxDrawdown(increasingPrices);
    EXPECT_DOUBLE_EQ(zeroDrawdown, 0.0);
    
    // 单调下跌的情况
    std::vector<double> decreasingPrices = {100.0, 90.0, 80.0, 70.0, 60.0};
    double maxDrawdown = RiskMetrics::calculateMaxDrawdown(decreasingPrices);
    EXPECT_NEAR(maxDrawdown, 0.4, 0.001);  // 从100跌到60，最大回撤40%
}

TEST_F(RiskMetricsTest, PortfolioRiskMetrics) {
    // 测试投资组合风险计算
    std::vector<std::vector<double>> assetReturns = {
        {0.01, 0.02, -0.01, 0.03},
        {-0.01, 0.01, 0.02, -0.02},
        {0.02, -0.01, 0.01, 0.02}
    };
    
    std::vector<double> weights = {0.4, 0.3, 0.3};
    
    auto portfolioRisk = RiskMetrics::calculatePortfolioRisk(assetReturns, weights, 0.95);
    
    EXPECT_EQ(portfolioRisk.size(), 3);  // VaR, CVaR, Volatility
    EXPECT_TRUE(std::isfinite(portfolioRisk[0]));  // VaR
    EXPECT_TRUE(std::isfinite(portfolioRisk[1]));  // CVaR
    EXPECT_TRUE(std::isfinite(portfolioRisk[2]));  // Volatility
    
    // 无效输入测试
    std::vector<double> invalidWeights = {0.5, 0.5};
    EXPECT_THROW(RiskMetrics::calculatePortfolioRisk(assetReturns, invalidWeights, 0.95), 
                 std::invalid_argument);
}

class ConvergenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 生成一些测试数据
        for (int i = 0; i < 1000; ++i) {
            testData.push_back(100.0 + 10.0 * sin(i * 0.1) + (rand() % 100) * 0.01);
        }
    }
    
    std::vector<double> testData;
};

TEST_F(ConvergenceTest, StandardErrorCalculation) {
    double se = Convergence::standardError(testData);
    
    EXPECT_GT(se, 0.0);
    EXPECT_TRUE(std::isfinite(se));
    
    // 常数序列的标准误应该为0
    std::vector<double> constantData(100, 50.0);
    double zeroSE = Convergence::standardError(constantData);
    EXPECT_DOUBLE_EQ(zeroSE, 0.0);
}

TEST_F(ConvergenceTest, ConvergenceCheck) {
    // 测试收敛性检查
    bool converged = Convergence::checkConvergence(testData, 10, 0.01);
    
    // 结果应该是布尔值
    EXPECT_TRUE(converged == true || converged == false);
    
    // 常数序列应该收敛
    std::vector<double> constantData(100, 50.0);
    bool constantConverged = Convergence::checkConvergence(constantData, 5, 0.01);
    EXPECT_TRUE(constantConverged);
}

TEST_F(ConvergenceTest, EffectiveSampleSize) {
    double ess = Convergence::effectiveSampleSize(testData);
    
    EXPECT_GT(ess, 0.0);
    EXPECT_LE(ess, testData.size());
    
    // 独立数据应该接近样本大小
    std::vector<double> independentData;
    for (int i = 0; i < 100; ++i) {
        independentData.push_back(rand() * 1.0 / RAND_MAX);
    }
    double independentESS = Convergence::effectiveSampleSize(independentData);
    EXPECT_NEAR(independentESS, independentData.size(), 10.0);
}

TEST_F(ConvergenceTest, MonteCarloStandardError) {
    double mcse = Convergence::monteCarloStandardError(testData);
    
    EXPECT_GE(mcse, 0.0);
    EXPECT_TRUE(std::isfinite(mcse));
}

TEST_F(ConvergenceTest, ConvergenceRateEstimation) {
    auto rates = Convergence::estimateConvergenceRate(testData, 100);
    
    // 应该有多个收敛率估计
    EXPECT_FALSE(rates.empty());
    
    // 收敛率应该递减或稳定
    for (size_t i = 1; i < rates.size(); ++i) {
        EXPECT_GE(rates[i-1], rates[i]);  // 随着批次增多，标准误应该减小
    }
}

// 测试大数定律
TEST(ConvergenceLawTest, LawOfLargeNumbers) {
    std::vector<double> samples;
    double trueMean = 0.5;
    
    // 生成大量样本
    for (int i = 0; i < 10000; ++i) {
        samples.push_back(trueMean + (rand() * 1.0 / RAND_MAX - 0.5) * 0.1);
    }
    
    double sampleMean = Statistics::mean(samples);
    double standardError = Convergence::standardError(samples);
    
    // 样本均值应该接近真实均值
    EXPECT_NEAR(sampleMean, trueMean, 3 * standardError);
}

// 测试中心极限定理
TEST(ConvergenceTheoremTest, CentralLimitTheorem) {
    std::vector<double> sampleMeans;
    
    // 生成多个样本的均值
    for (int sample = 0; sample < 1000; ++sample) {
        std::vector<double> observations;
        for (int i = 0; i < 100; ++i) {
            observations.push_back(rand() * 1.0 / RAND_MAX);
        }
        sampleMeans.push_back(Statistics::mean(observations));
    }
    
    // 样本均值应该近似正态分布
    double meansMean = Statistics::mean(sampleMeans);
    double meansStdDev = Statistics::standardDeviation(sampleMeans);
    
    EXPECT_NEAR(meansMean, 0.5, 0.05);
    EXPECT_NEAR(meansStdDev, 1.0 / sqrt(12 * 100), 0.01);  // 均匀分布的标准误
}

// 性能测试：大数据量下的统计计算
TEST(StatisticsPerformanceTest, LargeDatasetPerformance) {
    std::vector<double> largeData;
    const int N = 1000000;  // 100万数据点
    
    for (int i = 0; i < N; ++i) {
        largeData.push_back(100.0 + 10.0 * sin(i * 0.001));
    }
    
    // 测试各种统计量的计算速度
    auto start = std::chrono::high_resolution_clock::now();
    
    double mean = Statistics::mean(largeData);
    double median = Statistics::median(largeData);
    double stdDev = Statistics::standardDeviation(largeData);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_TRUE(std::isfinite(mean));
    EXPECT_TRUE(std::isfinite(median));
    EXPECT_TRUE(std::isfinite(stdDev));
    
    // 验证计算在合理时间内完成（< 500ms）
    EXPECT_LT(duration.count(), 500);
}

// 边界条件测试
TEST(StatisticsEdgeCasesTest, ExtremeValues) {
    // 测试极大值
    std::vector<double> largeValues = {1e100, 2e100, 3e100};
    EXPECT_NO_THROW(Statistics::mean(largeValues));
    
    // 测试极小值
    std::vector<double> smallValues = {1e-100, 2e-100, 3e-100};
    EXPECT_NO_THROW(Statistics::mean(smallValues));
    
    // 测试混合极大极小值
    std::vector<double> mixedValues = {-1e100, 0.0, 1e100};
    EXPECT_NO_THROW(Statistics::mean(mixedValues));
    
    // 测试NaN和Inf值
    std::vector<double> nanValues = {1.0, 2.0, NAN, 4.0};
    EXPECT_TRUE(std::isnan(Statistics::mean(nanValues)));
}
