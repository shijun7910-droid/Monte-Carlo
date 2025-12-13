#pragma once
#include <vector>
#include <random>

class RandomGenerator {
public:
    virtual ~RandomGenerator() = default;
    virtual std::vector<double> generateNormal(int n) = 0;
    virtual std::vector<std::vector<double>> generateNormalMatrix(int rows, int cols) = 0;
};

// Mersenne Twister 随机数生成器
class MersenneTwister : public RandomGenerator {
private:
    std::mt19937 generator;
    std::normal_distribution<double> distribution;
    
public:
    MersenneTwister(unsigned int seed = std::random_device{}());
    std::vector<double> generateNormal(int n) override;
    std::vector<std::vector<double>> generateNormalMatrix(int rows, int cols) override;
};

// 准随机数生成器 (Sobol序列)
class SobolGenerator : public RandomGenerator {
private:
    unsigned int dimension;
    unsigned int count;
    
public:
    SobolGenerator(unsigned int dimension = 1);
    std::vector<double> generateNormal(int n) override;
    std::vector<std::vector<double>> generateNormalMatrix(int rows, int cols) override;
    
private:
    double sobolSequence(unsigned int index, unsigned int dim);
    double inverseNormalCDF(double u);
};
