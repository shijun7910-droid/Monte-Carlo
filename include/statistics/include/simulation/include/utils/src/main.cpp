#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>

#include "models/GBM.h"
#include "random/RandomGenerator.h"
#include "simulation/MonteCarloSimulator.h"
#include "statistics/Statistics.h"
#include "utils/CSVWriter.h"

void printHelp() {
    std::cout << "Currency Monte Carlo Simulation\n";
    std::cout << "===============================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  ./currency_simulator [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -i, --initial FLOAT     Initial exchange rate (default: 75.0)\n";
    std::cout << "  -d, --drift FLOAT       Drift parameter (default: 0.05)\n";
    std::cout << "  -v, --volatility FLOAT  Volatility parameter (default: 0.2)\n";
    std::cout << "  -s, --simulations INT   Number of simulations (default: 10000)\n";
    std::cout << "  -t, --steps INT         Number of time steps (default: 252)\n";
    std::cout << "  -dt, --timestep FLOAT   Time step in years (default: 1.0/252)\n";
    std::cout << "  -o, --output STRING     Output filename prefix (default: simulation)\n";
    std::cout << "  --seed INT              Random seed (default: 12345)\n";
    std::cout << "  --threads INT           Number of threads (default: auto)\n";
    std::cout << "  --save-paths            Save all simulation paths\n";
    std::cout << "  --batch-size INT        Batch size for memory efficiency (default: 1000)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  ./currency_simulator -i 75.0 -d 0.05 -v 0.2 -s 10000\n";
    std::cout << "  ./currency_simulator --initial 1.10 --drift 0.02 --volatility 0.15\n";
}

struct CommandLineOptions {
    double initialPrice = 75.0;
    double drift = 0.05;
    double volatility = 0.2;
    int numSimulations = 10000;
    int numSteps = 252;
    double timeStep = 1.0 / 252.0;
    std::string outputPrefix = "simulation";
    unsigned int seed = 12345;
    int numThreads = 0;  // 0表示自动检测
    bool savePaths = false;
    int batchSize = 1000;
};

CommandLineOptions parseArguments(int argc, char* argv[]) {
    CommandLineOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp();
            exit(0);
        }
        else if (arg == "-i" || arg == "--initial") {
            if (i + 1 < argc) options.initialPrice = std::stod(argv[++i]);
        }
        else if (arg == "-d" || arg == "--drift") {
            if (i + 1 < argc) options.drift = std::stod(argv[++i]);
        }
        else if (arg == "-v" || arg == "--volatility") {
            if (i + 1 < argc) options.volatility = std::stod(argv[++i]);
        }
        else if (arg == "-s" || arg == "--simulations") {
            if (i + 1 < argc) options.numSimulations = std::stoi(argv[++i]);
        }
        else if (arg == "-t" || arg == "--steps") {
            if (i + 1 < argc) options.numSteps = std::stoi(argv[++i]);
        }
        else if (arg == "-dt" || arg == "--timestep") {
            if (i + 1 < argc) options.timeStep = std::stod(argv[++i]);
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) options.outputPrefix = argv[++i];
        }
        else if (arg == "--seed") {
            if (i + 1 < argc) options.seed = std::stoi(argv[++i]);
        }
        else if (arg == "--threads") {
            if (i + 1 < argc) options.numThreads = std::stoi(argv[++i]);
        }
        else if (arg == "--save-paths") {
            options.savePaths = true;
        }
        else if (arg == "--batch-size") {
            if (i + 1 < argc) options.batchSize = std::stoi(argv[++i]);
        }
    }
    
    return options;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "No arguments provided. Using default parameters.\n";
        std::cout << "Use -h or --help for usage information.\n\n";
    }
    
    CommandLineOptions options = parseArguments(argc, argv);
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n=== Currency Monte Carlo Simulation ===\n";
    std::cout << "Initial Price: " << options.initialPrice << "\n";
    std::cout << "Drift (μ): " << options.drift << "\n";
    std::cout << "Volatility (σ): " << options.volatility << "\n";
    std::cout << "Number of Simulations: " << options.numSimulations << "\n";
    std::cout << "Number of Steps: " << options.numSteps << "\n";
    std::cout << "Time Step: " << options.timeStep << " years\n";
    std::cout << "Random Seed: " << options.seed << "\n";
    
#ifdef _OPENMP
    if (options.numThreads > 0) {
        omp_set_num_threads(options.numThreads);
    }
    int maxThreads = omp_get_max_threads();
    std::cout << "Using OpenMP with " 
              << (options.numThreads > 0 ? options.numThreads : maxThreads) 
              << " thread(s)\n";
#endif
    
    std::cout << std::endl;
    
    try {
        // 1. 创建模型
        auto gbm = std::make_shared<GBM>(
            options.initialPrice, 
            options.drift, 
            options.volatility
        );
        
        // 2. 创建随机数生成器
        auto randomGen = std::make_shared<NormalGenerator>(0.0, 1.0, options.seed);
        
        // 3. 创建模拟器
        MonteCarloSimulator simulator(gbm, randomGen);
        
        if (options.numThreads > 0) {
            simulator.setNumThreads(options.numThreads);
        }
        
        // 4. 运行模拟
        std::cout << "Running Monte Carlo simulation...\n";
        
        SimulationResult result;
        if (options.numSimulations > 10000 && options.batchSize > 0) {
            std::cout << "Using batch processing with batch size: " 
                      << options.batchSize << "\n";
            result = simulator.runSimulationBatch(
                options.numSimulations,
                options.numSteps,
                options.timeStep,
                options.batchSize
            );
        } else {
            result = simulator.runSimulation(
                options.numSimulations,
                options.numSteps,
                options.timeStep
            );
        }
        
        // 5. 显示结果
        std::cout << "\n=== Simulation Results ===\n";
        std::cout << "Execution Time: " << result.executionTime << " seconds\n";
        std::cout << "Simulations per second: " 
                  << options.numSimulations / result.executionTime << "\n";
        
        Statistics::printSummary(result.summary, "Final Price Distribution");
        
        // 6. 保存结果到文件
        std::string timestamp = std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        std::string summaryFile = options.outputPrefix + "_summary_" + timestamp + ".csv";
        std::vector<double> prices = result.finalPrices;
        CSVWriter::writeSummary(prices, summaryFile);
        
        std::string finalPricesFile = options.outputPrefix + "_final_prices_" + timestamp + ".csv";
        std::ofstream fpFile(finalPricesFile);
        if (fpFile.is_open()) {
            fpFile << "FinalPrice\n";
            for (double price : prices) {
                fpFile << std::setprecision(10) << price << "\n";
            }
            fpFile.close();
            std::cout << "Final prices saved to: " << finalPricesFile << "\n";
        }
        
        if (options.savePaths && !result.paths.empty()) {
            std::string pathsFile = options.outputPrefix + "_paths_" + timestamp + ".csv";
            CSVWriter::writePaths(result.paths, pathsFile);
            std::cout << "All simulation paths saved to: " << pathsFile << "\n";
        }
        
        // 7. 额外分析
        std::cout << "\n=== Additional Analysis ===\n";
        
        // 计算百分位数
        std::vector<double> percentiles = {0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99};
        auto percentileValues = simulator.calculatePercentiles(prices, percentiles);
        
        std::cout << "\nPercentiles:\n";
        for (size_t i = 0; i < percentiles.size(); ++i) {
            std::cout << "  " << (percentiles[i] * 100) << "th percentile: " 
                      << percentileValues[i] << "\n";
        }
        
        // 计算概率
        double targetPrice = options.initialPrice * 1.1;  // 价格上涨10%
        int countAbove = 0;
        for (double price : prices) {
            if (price >= targetPrice) ++countAbove;
        }
        double probability = static_cast<double>(countAbove) / prices.size();
        
        std::cout << "\nProbability Analysis:\n";
        std::cout << "  Probability of price >= " << targetPrice << ": " 
                  << (probability * 100) << "%\n";
        
        double expectedPrice = Statistics::mean(prices);
        std::cout << "  Expected final price: " << expectedPrice << "\n";
        std::cout << "  Expected return: " 
                  << ((expectedPrice - options.initialPrice) / options.initialPrice * 100) 
                  << "%\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nSimulation completed successfully!\n";
    return 0;
}
