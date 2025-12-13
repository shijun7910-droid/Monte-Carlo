#include "RandomGenerator.h"
#include <cmath>
#include <algorithm>

// MersenneTwister 实现
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

// SobolGenerator 实现
SobolGenerator::SobolGenerator(unsigned int dim) 
    : dimension(dim), count(0) {}

std::vector<double> SobolGenerator::generateNormal(int n) {
    std::vector<double> result(n);
    for (int i = 0; i < n; i++) {
        double u = sobolSequence(count++, 0);
        result[i] = inverseNormalCDF(u);
    }
    return result;
}

std::vector<std::vector<double>> SobolGenerator::generateNormalMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double u = sobolSequence(count, j);
            matrix[i][j] = inverseNormalCDF(u);
        }
        count++;
    }
    return matrix;
}

// Sobol序列生成函数
double SobolGenerator::sobolSequence(unsigned int index, unsigned int dim) {
    // 简化的Sobol序列实现
    static const unsigned int max_dim = 10;
    static const unsigned int directions[max_dim] = {
        0x80000000, 0x40000000, 0x20000000, 0x10000000,
        0x08000000, 0x04000000, 0x02000000, 0x01000000,
        0x00800000, 0x00400000
    };
    
    unsigned int result = 0;
    for (unsigned int i = 0; i < 32; i++) {
        if (index & (1 << i)) {
            result ^= directions[dim % max_dim] >> i;
        }
    }
    
    return result / 4294967296.0;  // 除以 2^32
}

// 逆正态分布CDF（近似）
double SobolGenerator::inverseNormalCDF(double u) {
    // 使用Beasley-Springer-Moro近似
    static const double a[4] = {
        2.50662823884,
        -18.61500062529,
        41.39119773534,
        -25.44106049637
    };
    
    static const double b[4] = {
        -8.47351093090,
        23.08336743743,
        -21.06224101826,
        3.13082909833
    };
    
    static const double c[9] = {
        0.3374754822726147,
        0.9761690190917186,
        0.1607979714918209,
        0.0276438810333863,
        0.0038405729373609,
        0.0003951896511919,
        0.0000321767881768,
        0.0000002888167364,
        0.0000003960315187
    };
    
    double x = u - 0.5;
    if (fabs(x) < 0.42) {
        double y = x * x;
        return x * (((a[3] * y + a[2]) * y + a[1]) * y + a[0]) /
                   ((((b[3] * y + b[2]) * y + b[1]) * y + b[0]) * y + 1.0);
    } else {
        double y = (x > 0) ? 1.0 - u : u;
        double t = sqrt(-2.0 * log(y));
        double z = t - ((c[0] * t + c[1]) * t + c[2]) /
                       (((c[3] * t + c[4]) * t + c[5]) * t + 1.0);
        return (x > 0) ? z : -z;
    }
}
