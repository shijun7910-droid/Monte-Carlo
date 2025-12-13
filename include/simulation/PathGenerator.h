#ifndef PATH_GENERATOR_H
#define PATH_GENERATOR_H

#include <vector>
#include <memory>
#include "models/StochasticModel.h"
#include "random/RandomGenerator.h"

class PathGenerator {
private:
    std::shared_ptr<StochasticModel> model_;
    std::shared_ptr<RandomGenerator> randomGenerator_;
    
public:
    PathGenerator(std::shared_ptr<StochasticModel> model,
                 std::shared_ptr<RandomGenerator> randomGen)
        : model_(model), randomGenerator_(randomGen) {}
    
    // Generate a single path
    std::vector<double> generatePath(
        double initialPrice,
        int steps,
        double dt
    ) {
        std::vector<double> randoms = randomGenerator_->generateVector(steps);
        return model_->simulatePath(initialPrice, steps, dt, randoms);
    }
    
    //
