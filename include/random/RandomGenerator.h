#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <vector>
#include <random>
#include <memory>

class RandomGenerator {
public:
    virtual ~RandomGenerator() = default;
    
    virtual double generate() = 0;
    virtual std::vector<double> generateVector(int size) = 0;
    virtual void setSeed(unsigned int seed) = 0;
    
    static std::shared_ptr<RandomGenerator> createNormalGenerator(
        double mean = 0.0, 
        double stddev = 1.0
    );
    
    static std::shared_ptr<RandomGenerator> createUniformGenerator(
        double min = 0.0, 
        double max = 1.0
    );
};

class NormalGenerator : public RandomGenerator {
private:
    std::mt19937 generator_;
    std::normal_distribution<double> distribution_;
    
public:
    NormalGenerator(double mean = 0.0, double stddev = 1.0, 
                   unsigned int seed = std::random_device{}())
        : generator_(seed), distribution_(mean, stddev) {}
    
    double generate() override {
        return distribution_(generator_);
    }
    
    std::vector<double> generateVector(int size) override {
        std::vector<double> result(size);
        for (int i = 0; i < size; ++i) {
            result[i] = distribution_(generator_);
        }
        return result;
    }
    
    void setSeed(unsigned int seed) override {
        generator_.seed(seed);
    }
};

#endif // RANDOM_GENERATOR_H
