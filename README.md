# Currency Simulation Using Monte Carlo Method

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake 3.10+](https://img.shields.io/badge/CMake-3.10+-064f8c.svg)](https://cmake.org/)
[![Build Passing](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/your-username/currency-monte-carlo/actions)
[![OpenMP Enabled](https://img.shields.io/badge/OpenMP-enabled-blue.svg)](https://www.openmp.org/)

A C++ implementation for simulating future currency exchange rates using Monte Carlo methods with parallel computing capabilities.

author：Дай Шицзюнь
---

## 📋 Table of Contents
- [✨ Features](#-features)
- [📦 Requirements](#-requirements)
- [🛠 Installation & Build](#-installation--build)
- [🚀 Usage](#-usage)
- [📁 Project Structure](#-project-structure)
- [📈 Examples](#-examples)
- [🧪 Testing](#-testing)
- [📄 License](#-license)

---

## ✨ Features
- **Multiple Stochastic Models**: GBM, Vasicek, Hull-White, and more
- **High Performance**: Parallel computation with OpenMP
- **Flexible Configuration**: JSON-based configuration files
- **Multiple Output Formats**: CSV, JSON, and binary formats
- **Visualization Tools**: Python scripts for result plotting
- **Statistical Analysis**: VaR, CVaR, confidence intervals
- **Modular Design**: Easy to extend with new models

---

## 📦 Requirements

### Essential
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.10 or higher
- Git

### Libraries
- **Eigen3** (linear algebra operations)
- **OpenMP** (optional, for parallel computation)
- **nlohmann/json** (JSON parsing and generation)

### Optional
- **Python 3.7+** (for visualization scripts)
- **Matplotlib** (for plotting graphs)
- **NumPy** (for data analysis)

---

## 🛠 Installation & Build

### 1. Clone the Repository
```bash
git clone https://github.com/your-username/currency-monte-carlo.git
cd currency-monte-carlo
```

### 2. Install Dependencies
Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libeigen3-dev libomp-dev python3 python3-pip
pip3 install matplotlib numpy pandas
```
macOS
```bash
brew install cmake eigen openmp python3
pip3 install matplotlib numpy pandas
```
Windows
```bash
vcpkg install eigen3
```
### 3. Build the Project
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release --parallel 4
```
### 4. Run Quick Test
```bash
# Navigate back to project root
cd ..

# Run a simple simulation
./build/monte_carlo_simulator --help
```
## 🚀 Usage
Basic Simulation
```bash
# Using configuration file
./build/monte_carlo_simulator --config configs/gbm_config.json

# Using command line arguments
./build/monte_carlo_simulator \
  --model GBM \
  --initial-price 75.0 \
  --drift 0.05 \
  --volatility 0.2 \
  --time-horizon 30 \
  --simulations 10000 \
  --output results/simulation.csv
```
Configuration File Example
Create configs/usd_rub_simulation.json:
```json
{
  "simulation": {
    "model": "GBM",
    "parameters": {
      "initial_price": 75.0,
      "drift": 0.05,
      "volatility": 0.2,
      "risk_free_rate": 0.03
    },
    "simulation_params": {
      "num_simulations": 10000,
      "num_steps": 252,
      "time_horizon_days": 365
    },
    "output": {
      "format": "csv",
      "filename": "results/usd_rub_forecast.csv",
      "save_raw_paths": false,
      "confidence_levels": [0.95, 0.99]
    },
    "random": {
      "seed": 42,
      "generator": "mt19937"
    }
  }
}
```
Command Line Reference
```bash
./monte_carlo_simulator [OPTIONS]

Required Options (if no config file):
  -i, --initial-price FLOAT    Initial exchange rate
  -d, --drift FLOAT            Drift parameter (mean return)
  -v, --volatility FLOAT       Volatility parameter

Simulation Parameters:
  -s, --simulations INT        Number of simulations (default: 10000)
  -t, --time-horizon INT       Time horizon in days (default: 30)
  -n, --num-steps INT          Number of time steps (default: 252)
  -m, --model STRING           Model type: GBM, Vasicek, HullWhite (default: GBM)

Output Options:
  -o, --output FILE            Output file path
  --format STRING              Output format: csv, json, binary (default: csv)
  --confidence FLOAT           Confidence level for intervals (default: 0.95)

Performance:
  --threads INT                Number of threads for OpenMP (default: auto)
  --seed INT                   Random seed for reproducibility

Miscellaneous:
  -c, --config FILE            Configuration file (overrides other options)
  -h, --help                   Show this help message
  --version                    Show version information
```

## 📁 Project Structure
```bash
currency-monte-carlo/
├── CMakeLists.txt              # CMake build configuration
├── README.md                   # This file
├── LICENSE                     # MIT License
│
├── include/                    # Header files
│   ├── models/                 # Pricing models
│   │   ├── StochasticModel.h
│   │   ├── GBM.h
│   │   ├── Vasicek.h
│   │   └── HullWhite.h
│   │
│   ├── random/                 # Random number generation
│   │   ├── RandomGenerator.h
│   │   ├── NormalGenerator.h
│   │   └── SobolGenerator.h
│   │
│   ├── statistics/             # Statistical functions
│   │   ├── Statistics.h
│   │   ├── RiskMetrics.h
│   │   └── Convergence.h
│   │
│   ├── simulation/             # Simulation engine
│   │   ├── MonteCarloSimulator.h
│   │   ├── PathGenerator.h
│   │   └── ResultAnalyzer.h
│   │
│   └── utils/                  # Utility functions
│       ├── CSVWriter.h
│       ├── JSONConfig.h
│       ├── Timer.h
│       └── Logger.h
│
├── src/                        # Source code
│   ├── main.cpp                # Main entry point
│   ├── models/
│   ├── random/
│   ├── statistics/
│   ├── simulation/
│   └── utils/
│
├── tests/                      # Unit tests
│   ├── unit_tests.cpp
│   ├── test_models.cpp
│   ├── test_statistics.cpp
│   └── CMakeLists.txt
│
├── examples/                   # Example programs
│   ├── basic_simulation.cpp
│   ├── multi_currency.cpp
│   ├── risk_analysis.cpp
│   └── CMakeLists.txt
│
├── scripts/                    # Python scripts
│   ├── visualize.py            # Result visualization
│   ├── analyze_results.py      # Statistical analysis
│   ├── plot_distribution.py    # Distribution plotting
│   └── requirements.txt        # Python dependencies
│
├── configs/                    # Configuration files
    ├── gbm_config.json
    ├── vasicek_config.json
    ├── eur_usd.json
    └── usd_rub.json

## 📈 Examples
Example 1: Basic USD/RUB Forecast
```bush
#include <iostream>
#include "models/GBM.h"
#include "simulation/MonteCarloSimulator.h"
#include "statistics/Statistics.h"

int main() {
    // Create GBM model for USD/RUB
    GBM model(75.0,     // Initial exchange rate
              0.05,     // Drift (5% annual return)
              0.2,      // Volatility (20% annual)
              0.03);    // Risk-free rate (3%);
    
    // Configure Monte Carlo simulator
    MonteCarloSimulator simulator(model);
    simulator.setSeed(12345);          // For reproducibility
    simulator.setNumThreads(4);        // Use 4 CPU cores
    
    // Run simulation
    auto results = simulator.runSimulation(
        10000,    // 10,000 simulations
        252,      // 252 trading days (1 year)
        365.0/252 // Time step (1 day)
    );
    
    // Analyze results
    ResultAnalyzer analyzer(results);
    
    // Calculate statistics
    auto stats = analyzer.calculateStatistics();
    
    // Print results
    std::cout << "Simulation Results:\n";
    std::cout << "===================\n";
    std::cout << "Initial Rate: " << model.getInitialPrice() << "\n";
    std::cout << "Mean Forecast: " << stats.mean << "\n";
    std::cout << "Median Forecast: " << stats.median << "\n";
    std::cout << "Std Deviation: " << stats.stdDev << "\n";
    std::cout << "95% Confidence Interval: [" 
              << stats.confidenceInterval95.first << ", "
              << stats.confidenceInterval95.second << "]\n";
    std::cout << "Value at Risk (95%): " << stats.var95 << "\n";
    std::cout << "Conditional VaR (95%): " << stats.cvar95 << "\n";
    
    // Save results to CSV
    results.saveToCSV("usd_rub_forecast.csv");
    
    return 0;
}
```
Example 2: Multi-Currency Simulation
```
#include <vector>
#include "models/GBM.h"
#include "simulation/MultiAssetSimulator.h"

int main() {
    // Create models for different currency pairs
    std::vector<std::shared_ptr<StochasticModel>> models = {
        std::make_shared<GBM>(1.10, 0.02, 0.15, 0.01),  // EUR/USD
        std::make_shared<GBM>(1.30, 0.03, 0.18, 0.01),  // GBP/USD
        std::make_shared<GBM>(110.0, 0.01, 0.12, 0.01)  // USD/JPY
    };
    
    // Correlation matrix
    Eigen::MatrixXd correlation(3, 3);
    correlation << 1.0, 0.7, 0.3,
                   0.7, 1.0, 0.4,
                   0.3, 0.4, 1.0;
    
    // Create multi-asset simulator
    MultiAssetSimulator simulator(models, correlation);
    
    // Run correlated simulation
    auto results = simulator.run(5000, 252);
    
    // Calculate portfolio statistics
    auto portfolioStats = results.calculatePortfolioStatistics();
    
    return 0;
}
```
## 🧪 Testing
Running Tests
```bush
# Build and run all tests
cd build
cmake -DBUILD_TESTS=ON ..
make
ctest --output-on-failure

# Run specific test
./tests/unit_tests --gtest_filter="GBMTest.*"

# Run with valgrind for memory checking
valgrind --leak-check=full ./tests/unit_tests
```
Test Coverage
```bush
# Generate coverage report
mkdir build-coverage && cd build-coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make
./tests/unit_tests
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```
## 📄 License
This project is licensed under the MIT License - see the LICENSE file for details
