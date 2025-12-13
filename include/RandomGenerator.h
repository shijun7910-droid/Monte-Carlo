#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <vector>
#include <random>  // 需要包含random头文件

class RandomGenerator {
public:
    virtual ~RandomGenerator() = default;
    virtual std::vector<double> generateNormal(int n) = 0;
    virtual std::vector<std::vector<double>> generateNormalMatrix(int rows, int cols) = 0;
};

class MersenneTwister : public RandomGenerator {
private:
    std::mt19937 generator;
    std::normal_distribution<double> distribution;
    
public:
    MersenneTwister(unsigned int seed = std::random_device{}());
    std::vector<double> generateNormal(int n) override;
    std::vector<std::vector<double>> generateNormalMatrix(int rows, int cols) override;
};

#endif // RANDOMGENERATOR_H
