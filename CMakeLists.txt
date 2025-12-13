cmake_minimum_required(VERSION 3.10)
project(CurrencyMonteCarlo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 可执行文件
add_executable(currency_mc
    src/main.cpp
    src/models/GBM.cpp
    src/random/RandomGenerator.cpp
    src/utils/CSVWriter.cpp
)

# 包含目录
target_include_directories(currency_mc PRIVATE include)

# 示例
add_executable(example
    examples/example.cpp
    src/models/GBM.cpp
    src/random/RandomGenerator.cpp
    src/utils/CSVWriter.cpp
)
target_include_directories(example PRIVATE include)
