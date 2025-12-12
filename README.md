🏗️ 技术栈
C++ 20 - 主要编程语言

Eigen 3.4+ - 线性代数计算库

libcurl - HTTP客户端库，用于数据获取

Matplot++ - 数据可视化库 (或可选的gnuplot接口)

CMake 3.16+ - 跨平台构建系统

Google Test - 单元测试框架 (可选)

📁 项目结构
text
exchange-monte-carlo-cpp/
├── CMakeLists.txt          # CMake构建配置
├── src/                    # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── data/              # 数据处理模块
│   │   ├── DataFetcher.cpp
│   │   └── DataFetcher.h
│   ├── simulation/        # 蒙特卡洛模拟模块
│   │   ├── MonteCarloSimulator.cpp
│   │   └── MonteCarloSimulator.h
│   ├── risk/             # 风险分析模块
│   │   ├── RiskAnalyzer.cpp
│   │   └── RiskAnalyzer.h
│   ├── visualization/    # 可视化模块
│   │   ├── Visualizer.cpp
│   │   └── Visualizer.h
│   └── utils/            # 工具函数
│       ├── Statistics.cpp
│       └── Statistics.h
├── tests/                 # 单元测试
│   ├── test_simulation.cpp
│   └── test_risk.cpp
├── examples/              # 使用示例
│   └── basic_example.cpp
├── data/                  # 数据存储目录
├── docs/                  # 文档
└── third_party/          # 第三方库
🔧 安装与配置
前提条件
编译器: GCC 11+, Clang 14+, 或 MSVC 2022+

CMake: 3.16 或更高版本

依赖库:

Eigen3 (线性代数)

libcurl (数据获取)

Matplot++ 或 gnuplot (可视化)

Ubuntu/Debian 安装
bash
# 安装系统依赖
sudo apt-get update
sudo apt-get install -y build-essential cmake libeigen3-dev libcurl4-openssl-dev

# 克隆项目
git clone https://github.com/shijun7910-droid/Monte-Carlo-Cpp.git
cd Monte-Carlo-Cpp

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
macOS 安装
bash
# 使用Homebrew安装依赖
brew install cmake eigen curl

# 克隆和构建（步骤同上）
git clone https://github.com/shijun7910-droid/Monte-Carlo-Cpp.git
cd Monte-Carlo-Cpp
mkdir build && cd build
cmake ..
make
Windows 安装 (使用vcpkg)
powershell
# 安装vcpkg（如果尚未安装）
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# 安装依赖
.\vcpkg install eigen3 curl:x64-windows

# 克隆项目
git clone https://github.com/shijun7910-droid/Monte-Carlo-Cpp.git
cd Monte-Carlo-Cpp

# 使用CMake配置
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg目录]\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
🚀 快速开始
基本使用示例
cpp
// examples/basic_example.cpp
#include "MonteCarloSimulator.h"
#include "RiskAnalyzer.h"
#include "Visualizer.h"

int main() {
    // 1. 创建模拟器实例
    MonteCarloSimulator simulator(10000); // 10000次模拟
    
    // 2. 设置参数（示例：USD/CNY）
    simulator.setParameters(6.50,  // 初始汇率
                           0.02,   // 年化波动率
                           0.01,   // 年化漂移率
                           252,    // 交易日数
                           1.0/252); // 时间步长（日）
    
    // 3. 运行模拟
    auto results = simulator.runSimulation();
    
    // 4. 风险分析
    RiskAnalyzer analyzer(results);
    double var95 = analyzer.calculateVaR(0.95);
    double var99 = analyzer.calculateVaR(0.99);
    
    std::cout << "95% VaR: " << var95 << std::endl;
    std::cout << "99% VaR: " << var99 << std::endl;
    
    // 5. 可视化结果
    Visualizer::plotPaths(results, "simulation_paths.png");
    Visualizer::plotDistribution(results, "return_distribution.png");
    
    return 0;
}
从Yahoo Finance获取数据
cpp
#include "DataFetcher.h"

int main() {
    DataFetcher fetcher;
    
    // 获取USD/CNY历史数据
    auto historicalData = fetcher.fetchFromYahooFinance(
        "CNY=X",           // 货币对
        "2023-01-01",      // 开始日期
        "2023-12-31",      // 结束日期
        DataFetcher::DAILY // 数据频率
    );
    
    // 计算历史波动率
    double volatility = Statistics::calculateVolatility(
        historicalData.getReturns()
    );
    
    return 0;
}
📊 主要功能
1. 数据获取模块
从Yahoo Finance获取历史汇率数据

支持多种数据频率（日、周、月）

CSV数据导入/导出功能

数据清洗和预处理

2. 蒙特卡洛模拟
基于几何布朗运动(GBM)的汇率模拟

支持多种随机数生成器

可配置的模拟次数和预测周期

并行计算优化

3. 风险分析
风险价值(VaR)计算（历史法、参数法、蒙特卡洛法）

条件风险价值(CVaR/ES)计算

压力测试和情景分析

敏感性分析

4. 结果可视化
模拟路径图

收益率分布直方图

VaR和CVaR可视化

风险度量报告生成

🧪 运行测试
bash
# 在构建目录中
cd build

# 启用测试选项重新配置
cmake .. -DBUILD_TESTS=ON
make

# 运行测试
ctest --output-on-failure

# 或直接运行测试可执行文件
./tests/test_simulation
./tests/test_risk
📈 性能特性
高性能计算: 使用Eigen库进行向量化计算

内存效率: 优化的大型矩阵操作

并行处理: 支持多线程模拟

缓存友好: 数据局部性优化

🔍 配置选项
CMake配置选项：

bash
cmake .. -DUSE_OPENMP=ON      # 启用OpenMP并行
         -DUSE_CUDA=OFF       # CUDA支持（如果可用）
         -DUSE_MPI=OFF        # MPI支持
         -DPLOT_BACKEND="gnuplot"  # 可视化后端选择
         -DBUILD_EXAMPLES=ON  # 构建示例程序
📝 输出格式
程序可以生成多种格式的输出：

CSV文件（用于进一步分析）

PNG/PDF图表

JSON格式报告

控制台统计摘要

🤝 贡献指南
Fork 本仓库

创建功能分支 (git checkout -b feature/AmazingFeature)

提交更改 (git commit -m 'Add some AmazingFeature')

推送到分支 (git push origin feature/AmazingFeature)

开启 Pull Request

📄 许可证
本项目采用 MIT 许可证 - 查看 LICENSE 文件了解详情。
