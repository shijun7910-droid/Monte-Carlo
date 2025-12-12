Exchange Rate Monte Carlo Simulator (C++)
A C++ implementation of Monte Carlo simulation for exchange rate forecasting and risk analysis.

Features
Monte Carlo simulation using Geometric Brownian Motion

Value at Risk (VaR) calculation

Historical data processing

Result visualization

Requirements
C++20 compatible compiler

CMake 3.16+

Eigen3 library

libcurl (for data fetching)

Quick Start
bash
# Clone repository
git clone https://github.com/shijun7910-droid/Monte-Carlo-Cpp.git
cd Monte-Carlo-Cpp

# Build project
mkdir build && cd build
cmake .. && make

# Run simulation
./exchange_simulator
Project Structure
text
src/
├── data/           # Data fetching and processing
├── simulation/     # Monte Carlo simulation engine
├── risk/          # Risk analysis (VaR, CVaR)
├── visualization/ # Plotting and chart generation
└── utils/         # Utility functions
Example Usage
cpp
#include "MonteCarloSimulator.h"

int main() {
    MonteCarloSimulator sim(10000); // 10,000 simulations
    sim.setParameters(6.5, 0.02, 0.01, 252);
    auto results = sim.runSimulation();
    
    // Analyze results...
    return 0;
}
License
MIT License
