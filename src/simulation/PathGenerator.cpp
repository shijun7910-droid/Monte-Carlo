#include "simulation/PathGenerator.h"
#include <stdexcept>
#include <cmath>

PathGenerator::PathGenerator(std::shared_ptr<StochasticModel> model,
                           std::shared_ptr<RandomGenerator> randomGen)
    : model_(model), randomGenerator_(randomGen) {
    
    if (!model_) {
        throw std::invalid_argument("Model cannot be null");
    }
    if (!randomGenerator_) {
        throw std::invalid_argument("Random generator cannot be null");
    }
}

std::vector<double> PathGenerator::generatePath(
    double initialPrice,
    int steps,
    double dt
) {
    if (steps <= 0) {
        throw std::invalid_argument("Number of steps must be positive");
    }
    if (dt <= 0.0) {
        throw std::invalid_argument("Time step must be positive");
    }
    if (initialPrice <= 0.0) {
        throw std::invalid_argument("Initial price must be positive");
    }
    
    std::vector<double> randoms = randomGenerator_->generateVector(steps);
    return model_->simulatePath(initialPrice, steps, dt, randoms);
}

std::vector<std::vector<double>> PathGenerator::generatePaths(
    int numPaths,
    double initialPrice,
    int steps,
    double dt
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    
    std::vector<std::vector<double>> paths(numPaths);
    
    for (int i = 0; i < numPaths; ++i) {
        paths[i] = generatePath(initialPrice, steps, dt);
    }
    
    return paths;
}

std::vector<std::vector<double>> PathGenerator::generatePathsAntithetic(
    int numPaths,
    double initialPrice,
    int steps,
    double dt
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    
    std::vector<std::vector<double>> paths(numPaths);
    
    for (int i = 0; i < numPaths; i += 2) {
        // Generate random numbers for first path
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

std::vector<double> PathGenerator::calculatePathStatistics(
    const std::vector<double>& path
) {
    if (path.empty()) {
        return {};
    }
    
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
    double maxPrice = path[0];
    for (double price : path) {
        if (price > maxPrice) maxPrice = price;
    }
    stats.push_back(maxPrice);
    
    // Minimum
    double minPrice = path[0];
    for (double price : path) {
        if (price < minPrice) minPrice = price;
    }
    stats.push_back(minPrice);
    
    // Final value
    stats.push_back(path.back());
    
    // Maximum drawdown
    double peak = path[0];
    double maxDrawdown = 0.0;
    for (double price : path) {
        if (price > peak) {
            peak = price;
        }
        double drawdown = (peak - price) / peak;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }
    }
    stats.push_back(maxDrawdown);
    
    return stats;
}

std::vector<double> PathGenerator::calculatePathReturns(const std::vector<double>& path) {
    if (path.size() < 2) {
        return {};
    }
    
    std::vector<double> returns(path.size() - 1);
    for (size_t i = 1; i < path.size(); ++i) {
        returns[i-1] = (path[i] - path[i-1]) / path[i-1];
    }
    
    return returns;
}

std::vector<double> PathGenerator::calculatePathLogReturns(const std::vector<double>& path) {
    if (path.size() < 2) {
        return {};
    }
    
    std::vector<double> logReturns(path.size() - 1);
    for (size_t i = 1; i < path.size(); ++i) {
        if (path[i-1] > 0.0 && path[i] > 0.0) {
            logReturns[i-1] = std::log(path[i] / path[i-1]);
        } else {
            logReturns[i-1] = 0.0;
        }
    }
    
    return logReturns;
}

// Generate paths with control variates (variance reduction technique)
std::vector<std::vector<double>> PathGenerator::generatePathsControlVariate(
    int numPaths,
    double initialPrice,
    int steps,
    double dt,
    const std::vector<double>& controlVariate
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    
    if (controlVariate.size() != static_cast<size_t>(steps)) {
        throw std::invalid_argument("Control variate size must match number of steps");
    }
    
    std::vector<std::vector<double>> paths(numPaths);
    
    for (int i = 0; i < numPaths; ++i) {
        // Generate random numbers
        std::vector<double> randoms = randomGenerator_->generateVector(steps);
        
        // Apply control variate adjustment
        std::vector<double> adjustedRandoms = randoms;
        for (int j = 0; j < steps; ++j) {
            // Simple control variate adjustment
            adjustedRandoms[j] = randoms[j] - 0.5 * controlVariate[j];
        }
        
        paths[i] = model_->simulatePath(initialPrice, steps, dt, adjustedRandoms);
    }
    
    return paths;
}

// Generate correlated paths for multiple assets
std::vector<std::vector<std::vector<double>>> PathGenerator::generateCorrelatedPaths(
    int numPaths,
    const std::vector<double>& initialPrices,
    int steps,
    double dt,
    const std::vector<std::vector<double>>& correlationMatrix
) {
    if (numPaths <= 0) {
        throw std::invalid_argument("Number of paths must be positive");
    }
    
    size_t numAssets = initialPrices.size();
    if (numAssets == 0) {
        throw std::invalid_argument("Initial prices cannot be empty");
    }
    
    if (correlationMatrix.size() != numAssets) {
        throw std::invalid_argument("Correlation matrix dimensions must match number of assets");
    }
    
    // Generate independent random numbers for each asset
    std::vector<std::vector<std::vector<double>>> allRandoms(
        numAssets, 
        std::vector<std::vector<double>>(numPaths, std::vector<double>(steps))
    );
    
    for (size_t asset = 0; asset < numAssets; ++asset) {
        for (int path = 0; path < numPaths; ++path) {
            allRandoms[asset][path] = randomGenerator_->generateVector(steps);
        }
    }
    
    // Apply correlation (simplified Cholesky decomposition)
    std::vector<std::vector<std::vector<double>>> correlatedPaths(
        numAssets,
        std::vector<std::vector<double>>(numPaths, std::vector<double>(steps))
    );
    
    // For simplicity, we'll use a simplified correlation approach
    // In production, use proper Cholesky decomposition
    for (int path = 0; path < numPaths; ++path) {
        for (int step = 0; step < steps; ++step) {
            for (size_t asset = 0; asset < numAssets; ++asset) {
                double correlatedRandom = 0.0;
                for (size_t j = 0; j < numAssets; ++j) {
                    correlatedRandom += correlationMatrix[asset][j] * allRandoms[j][path][step];
                }
                correlatedRandom /= numAssets;
                
                // Use the first asset's model for all assets (simplification)
                // In production, each asset would have its own model
                if (step == 0) {
                    correlatedPaths[asset][path][step] = model_->simulate(
                        initialPrices[asset], dt, correlatedRandom);
                } else {
                    correlatedPaths[asset][path][step] = model_->simulate(
                        correlatedPaths[asset][path][step-1], dt, correlatedRandom);
                }
            }
        }
    }
    
    return correlatedPaths;
}
