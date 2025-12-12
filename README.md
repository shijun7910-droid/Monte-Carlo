markdown
# Currency Simulation Using Monte Carlo Method

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-064f8c.svg)](https://cmake.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/your-username/currency-monte-carlo)
[![OpenMP](https://img.shields.io/badge/OpenMP-enabled-blue.svg)](https://www.openmp.org/)

A C++ implementation for simulating future currency exchange rates using Monte Carlo methods with parallel computing capabilities.

## 📋 Table of Contents

- [Features](#-features)
- [Requirements](#-requirements)
- [Installation & Build](#-installation--build)
- [Usage](#-usage)
- [Project Structure](#-project-structure)
- [Mathematical Models](#-mathematical-models)
- [Examples](#-examples)
- [Testing](#-testing)
- [Contributing](#-contributing)
- [License](#-license)

## ✨ Features

- **Multiple Stochastic Models**: GBM, Vasicek, Hull-White, and more
- **High Performance**: Parallel computation with OpenMP
- **Flexible Configuration**: JSON-based configuration files
- **Multiple Output Formats**: CSV, JSON, and binary formats
- **Visualization Tools**: Python scripts for result plotting
- **Statistical Analysis**: VaR, CVaR, confidence intervals
- **Modular Design**: Easy to extend with new models

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

## 🛠 Installation & Build

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/currency-monte-carlo.git
cd currency-monte-carlo
`git clone https://github.com/your-username/currency-monte-carlo.git`  
`cd currency-monte-carlo`

### 2. Install Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libeigen3-dev libomp-dev python3 python3-pip
pip3 install matplotlib numpy pandas
