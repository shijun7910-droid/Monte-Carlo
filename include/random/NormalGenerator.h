#ifndef NORMAL_GENERATOR_H
#define NORMAL_GENERATOR_H

#include "RandomGenerator.h"

// NormalGenerator is already defined in RandomGenerator.h
// This file provides additional factory functions

class RandomGeneratorFactory {
public:
    static std::shared_ptr<RandomGenerator> createNormalGenerator(
        double mean = 0.0, 
        double stddev = 1.0,
        unsigned int seed = 0
    ) {
        if (seed == 0) {
            seed = std::random_device{}();
        }
        return std::make_shared<NormalGenerator>(mean, stddev, seed);
    }
    
    static std::shared_ptr<RandomGenerator> createSobolGenerator(
        unsigned int dimension = 1
    ) {
        return std::make_shared<SobolGenerator>(dimension);
    }
};

#endif // NORMAL_GENERATOR_H
