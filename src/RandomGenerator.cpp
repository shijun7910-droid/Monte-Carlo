#include "../include/RandomGenerator.h"
#include <random>
#include <chrono>

// 构造函数
MersenneTwister::MersenneTwister(unsigned int seed) 
    : generator(seed), distribution(0.0, 1.0) {
    // 可选：输出种子信息用于调试
    // std::cout << "MersenneTwister initialized with seed: " << seed << std::endl;
}

// 生成n个正态分布随机数
std::vector<double> MersenneTwister::generateNormal(int n) {
    std::vector<double> result;
    result.reserve(n);  // 预分配空间以提高性能
    
    for (int i = 0; i < n; i++) {
        result.push_back(distribution(generator));
    }
    
    return result;
}

// 生成rows×cols矩阵的正态分布随机数
std::vector<std::vector<double>> MersenneTwister::generateNormalMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix;
    matrix.reserve(rows);  // 预分配行空间
    
    for (int i = 0; i < rows; i++) {
        std::vector<double> row;
        row.reserve(cols);  // 预分配列空间
        
        for (int j = 0; j < cols; j++) {
            row.push_back(distribution(generator));
        }
        
        matrix.push_back(row);
    }
    
    return matrix;
}
