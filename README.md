# Currency Simulation Using Monte Carlo Method

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake 3.10+](https://img.shields.io/badge/CMake-3.10+-064f8c.svg)](https://cmake.org/)
[![Build Passing](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/your-username/currency-monte-carlo/actions)
[![OpenMP Enabled](https://img.shields.io/badge/OpenMP-enabled-blue.svg)](https://www.openmp.org/)

A high-performance C++ implementation for simulating future currency exchange rates using Monte Carlo methods with parallel computing capabilities.

---

## 📋 Table of Contents
- [✨ Features](#-features)
- [📦 Requirements](#-requirements)
- [🛠 Installation & Build](#-installation--build)
- [🚀 Usage](#-usage)
- [📁 Project Structure](#-project-structure)
- [📊 Mathematical Models](#-mathematical-models)
- [📈 Examples](#-examples)
- [🧪 Testing](#-testing)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)
- [📞 Contact & Support](#-contact--support)
- [🙏 Acknowledgments](#-acknowledgments)
- [🚀 Roadmap](#-roadmap)

---

## ✨ Features
- **Multiple Stochastic Models**: GBM, Vasicek, Hull-White, Jump Diffusion
- **High Performance**: Parallel computation with OpenMP (10x+ speedup)
- **Flexible Configuration**: JSON-based configuration files
- **Multiple Output Formats**: CSV, JSON, and binary
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
| Library | Purpose | Installation |
|---------|---------|--------------|
| [Eigen3](https://eigen.tuxfamily.org/) | Linear algebra | `apt install libeigen3-dev`, `brew install eigen`, or `vcpkg install eigen3` |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing | Included in `third_party/` |
| OpenMP | Parallel computing | `apt install libomp-dev`, `brew install libomp` |

### Optional (for visualization)
- Python 3.7+
- `pip install matplotlib numpy pandas`

---

## 🛠 Installation & Build

### 1. Clone the Repository
```bash
git clone https://github.com/your-username/currency-monte-carlo.git
cd currency-monte-carlo
