# Currency Simulation Using Monte Carlo Method

A professional C++ library for simulating and analyzing currency exchange rate movements, supporting multiple stochastic processes and risk management features.

author：Дай Шицзюнь
---

## Core Features
Multiple Models: Geometric Brownian Motion (GBM), Mean Reversion (Vasicek)

Advanced RNG: Mersenne Twister pseudo-random number generator

Comprehensive Risk Analysis: VaR, CVaR, percentile analysis, probability statistics

Result Visualization: Python scripts for professional charts

User-Friendly: Interactive menus, multiple example programs

---

## Project Structure
```bash
Monte-Carlo-main/
├── main.cpp                 # Main program
├── example.cpp             # Collection of examples
├── all_in_one.cpp          # Single-file complete version
├── simple_test.cpp         # Minimal test version
├── CMakeLists.txt          # CMake build configuration
├── include/                # Header files
│   ├── MonteCarlo.h       # Main simulator interface
│   ├── CurrencyModel.h    # Financial model definitions
│   ├── RandomGenerator.h  # Random number generation
│   └── Statistics.h       # Statistical calculations
├── src/                   # Source implementations
└── scripts/              # Python visualization scripts
```
---

## Running Programs
```bash
g++ -std=c++17 -o2 -o main main.cpp src/MonteCarlo.cpp src/RandomGenerator.cpp src/CurrencyModel.cpp src/Statistics.cpp
./main
```

## Feature Demonstration
### 1. Basic GBM Simulation
```bash
// Create GBM model
auto model = std::make_unique<GBM>(0.02, 0.15);  // 2% return, 15% volatility
auto randomGen = std::make_unique<MersenneTwister>(42);

// Run simulation
MonteCarloSimulator simulator(std::move(model), std::move(randomGen),
                              10000, 252, 1.0);
auto results = simulator.runSimulation(0.92);  // Initial exchange rate
```
Sample Output:
```bash
=== GBM Model Simulation USD/EUR Exchange Rate ===
Mean: 0.9382
Standard Deviation: 0.1414
95% VaR: 0.7248
95% CVaR: 0.6798
Probability of increase: 51.60%
```
### 2. Mean Reversion Model
```bash
// Create Vasicek model
auto model = std::make_unique<Vasicek>(0.1, 0.05, 0.02);
auto results = simulator.runSimulation(0.06);  // Initial interest rate 6%
```

### 3. Risk Analysis
```bash
// Calculate risk metrics at different confidence levels
double var95 = Statistics::valueAtRisk(results, 0.95);
double cvar95 = Statistics::conditionalVaR(results, 0.95);
```

## Practical Applications
### Exchange Rate Forecasting
```bash
// Forecast USD/JPY rate after one year
double initialJPY = 110.50;  // 1 USD = 110.50 JPY
GBM usdJpyModel(0.01, 0.12); // 1% return, 12% volatility
// Run 10,000 simulations...
```
### Scenario Analysis
```bash
// Compare different volatility scenarios
vector<Scenario> scenarios = {
    {"Low Volatility", 0.02, 0.08},
    {"Medium Volatility", 0.02, 0.15},
    {"High Volatility", 0.02, 0.25}
};
```

## Validation Examples
### Sample Output Validation
```bash
Monte Carlo Simulation Test Program
====================================
1. Testing Random Number Generator:
10 random numbers: 0.5951 0.0418 0.8004 -0.0719 -1.0431 ...

2. Testing GBM Model:
GBM path (10 steps): 1.0295 1.0325 1.0734 1.0707 1.0199 ...

3. Running Monte Carlo Simulation:
Number of simulations: 1000
Mean final rate: 102.03
Standard deviation: 15.33
Probability of increase: 51.60%
```
##  License
MIT License - See LICENSE file for details
