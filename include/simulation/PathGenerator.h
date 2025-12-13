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
    
    // Generate multiple paths
    std::vector<std::vector<double>> generatePaths(
        int numPaths,
        double initialPrice,
        int steps,
        double dt
    ) {
        std::vector<std::vector<double>> paths(numPaths);
        
        for (int i = 0; i < numPaths; ++i) {
            paths[i] = generatePath(initialPrice, steps, dt);
        }
        
        return paths;
    }
    
    // Generate paths with antithetic variates (variance reduction)
    std::vector<std::vector<double>> generatePathsAntithetic(
        int numPaths,
        double initialPrice,
        int steps,
        double dt
    ) {
        std::vector<std::vector<double>> paths(numPaths);
        
        for (int i = 0; i < numPaths; i += 2) {
            // Generate random numbers
            std::vector<double> randoms = randomGenerator_->generateVector(steps);
            
            // First path
            paths[i] = model_->simulatePath(initialPrice, steps, dt, randoms);
            
            // Second path with antithetic variates (negative of randoms)
            if (i + 1 < numPaths) {
                std::vector<double> antitheticRandoms = randoms;
                for (double& r : antitheticRandoms) {
                    r = -r;
                }
                paths[i + 1] = model_->simulatePath(initialPrice, steps, dt, antitheticRandoms);
            }
        }
        
        return paths;
    }
    
    // Calculate path statistics
    std::vector<double> calculatePathStatistics(
        const std::vector<double>& path
    ) {
        if (path.empty()) return {};
        
        std::vector<double> stats;
        
        // Mean
        double sum = 0.0;
        for (double price : path) {
            sum += price;
        }
        stats.push_back(sum / path.size());
        
        // Standard deviation
        double mean = stats[0];
        double variance = 0.0;
        for (double price : path) {
            variance += (price - mean) * (price - mean);
        }
        stats.push_back(std::sqrt(variance / (path.size() - 1)));
        
        // Maximum
        stats.push_back(*std::max_element(path.begin(), path.end()));
        
        // Minimum
        stats.push_back(*std::min_element(path.begin(), path.end()));
        
        // Final value
        stats.push_back(path.back());
        
        return stats;
    }
    
    // Calculate path returns
    std::vector<double> calculatePathReturns(const std::vector<double>& path) {
        if (path.size() < 2) return {};
        
        std::vector<double> returns(path.size() - 1);
        for (size_t i = 1; i < path.size(); ++i) {
            returns[i-1] = (path[i] - path[i-1]) / path[i-1];
        }
        
        return returns;
    }
    
    // Calculate path log returns
    std::vector<double> calculatePathLogReturns(const std::vector<double>& path) {
        if (path.size() < 2) return {};
        
        std::vector<double> logReturns(path.size() - 1);
        for (size_t i = 1; i < path.size(); ++i) {
            logReturns[i-1] = std::log(path[i] / path[i-1]);
        }
        
        return logReturns;
    }
};

#endif // PATH_GENERATOR_H
