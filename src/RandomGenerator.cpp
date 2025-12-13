#include "../include/RandomGenerator.h"
#include <random>

MersenneTwister::MersenneTwister(unsigned int seed) 
    : generator(seed), distribution(0.0, 1.0) {}

std::vector<double> MersenneTwister::generateNormal(int n) {
    std::vector<double> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = distribution(generator);
    }
    return result;
}

std::vector<std::vector<double>> MersenneTwister::generateNormalMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = distribution(generator);
        }
    }
    return matrix;
}
