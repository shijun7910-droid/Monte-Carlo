#include "models/GBM.h"

// Implementation is already in the header file
// This file can be used for additional GBM-specific implementations

namespace {
    // Helper function for GBM parameter validation
    bool validateGBMParameters(double initialPrice, double drift, double volatility) {
        if (initialPrice <= 0.0) {
            std::cerr << "Error: Initial price must be positive\n";
            return false;
        }
        if (volatility < 0.0) {
            std::cerr << "Error: Volatility cannot be negative\n";
            return false;
        }
        return true;
    }
}

// Additional GBM utility functions can be added here
