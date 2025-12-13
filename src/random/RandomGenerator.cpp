#include "random/RandomGenerator.h"
#include <random>
#include <cmath>
#include <stdexcept>

// NormalGenerator implementation
NormalGenerator::NormalGenerator(double mean, double stddev, unsigned int seed)
    : generator_(seed), distribution_(mean, stddev) {
    
    if (stddev < 0.0) {
        throw std::invalid_argument("Standard deviation cannot be negative");
    }
}

double NormalGenerator::generate() {
    return distribution_(generator_);
}

std::vector<double> NormalGenerator::generateVector(int size) {
    if (size <= 0) {
        throw std::invalid_argument("Vector size must be positive");
    }
    
    std::vector<double> result(size);
    for (int i = 0; i < size; ++i) {
        result[i] = distribution_(generator_);
    }
    return result;
}

void NormalGenerator::setSeed(unsigned int seed) {
    generator_.seed(seed);
    // Reset distribution to ensure reproducibility
    distribution_.reset();
}

std::string NormalGenerator::getName() const {
    return "Normal Distribution Generator (μ=" + 
           std::to_string(distribution_.mean()) + 
           ", σ=" + 
           std::to_string(distribution_.stddev()) + ")";
}

// SobolGenerator implementation
SobolGenerator::SobolGenerator(unsigned int dimension) 
    : dimension_(dimension), count_(0) {
    
    if (dimension == 0) {
        throw std::invalid_argument("Dimension must be positive");
    }
    
    // Initialize direction numbers (simplified implementation)
    // In production, use a proper Sobol sequence library
    directionNumbers_.resize(dimension);
    for (unsigned int i = 0; i < dimension; ++i) {
        directionNumbers_[i] = static_cast<double>(i + 1) / (1 << 16);
    }
}

double SobolGenerator::generate() {
    // Simplified Sobol sequence implementation
    // For production, use a proper Sobol sequence generator
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    if (count_ < directionNumbers_.size()) {
        return directionNumbers_[count_++];
    } else {
        return dis(gen);
    }
}

std::vector<double> SobolGenerator::generateVector(int size) {
    if (size <= 0) {
        throw std::invalid_argument("Vector size must be positive");
    }
    
    std::vector<double> result(size);
    for (int i = 0; i < size; ++i) {
        result[i] = generate();
    }
    return result;
}

void SobolGenerator::setSeed(unsigned int seed) {
    // Sobol sequences don't use seeds in the same way as pseudo-random generators
    // We can use the seed to reset the counter
    count_ = seed % 1000;  // Simple reset mechanism
}

std::string SobolGenerator::getName() const {
    return "Sobol Sequence Generator (dimension=" + 
           std::to_string(dimension_) + ")";
}

unsigned int SobolGenerator::grayCode(unsigned int n) {
    return n ^ (n >> 1);
}

// RandomGeneratorFactory implementation
std::shared_ptr<RandomGenerator> RandomGeneratorFactory::createNormalGenerator(
    double mean, 
    double stddev,
    unsigned int seed
) {
    if (seed == 0) {
        seed = std::random_device{}();
    }
    return std::make_shared<NormalGenerator>(mean, stddev, seed);
}

std::shared_ptr<RandomGenerator> RandomGeneratorFactory::createSobolGenerator(
    unsigned int dimension
) {
    return std::make_shared<SobolGenerator>(dimension);
}
