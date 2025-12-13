#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

#include "models/GBM.h"
#include "models/Vasicek.h"
#include "models/HullWhite.h"
#include "random/RandomGenerator.h"
#include "simulation/MonteCarloSimulator.h"
#include "simulation/ResultAnalyzer.h"
#include "statistics/Statistics.h"
#include "statistics/RiskMetrics.h"
#include "utils/CSVWriter.h"
#include "utils/JSONConfig.h"
#include "utils/Timer.h"
#include "utils/Logger.h"

void printHelp() {
    std::cout << "Currency Monte Carlo Simulation\n";
    std::cout << "================================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  ./currency_simulator [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -i, --initial FLOAT     Initial exchange rate (default: 75.0)\n";
    std::cout << "  -d, --drift FLOAT       Drift parameter (default: 0.05)\n";
    std::cout << "  -v, --volatility FLOAT  Volatility parameter (default: 0.2)\n";
    std::cout << "  -m, --model STRING      Model type: GBM, Vasicek, HullWhite (default: GBM)\n";
    std::cout << "  -s, --simulations INT   Number of simulations (default: 10000)\n";
    std::cout << "  -t, --steps INT         Number of time steps (default: 252)\n";
    std::cout << "  -dt, --timestep FLOAT   Time step in years (default: 1.0/252)\n";
    std::cout << "  -o, --output STRING     Output filename prefix (default: simulation)\n";
    std::cout << "  --seed INT              Random seed (default: 12345)\n";
    std::cout << "  --threads INT           Number of threads (default: auto)\n";
    std::cout << "  --config FILE           Configuration file\n";
    std::cout << "  --save-paths            Save all simulation paths\n";
    std::cout << "  --batch-size INT        Batch size for memory efficiency (default: 1000)\n";
    std::cout << "  --risk-analysis         Perform detailed risk analysis\n";
    std::cout << "\nExamples:\n";
    std::cout << "  ./currency_simulator -i 75.0 -d 0.05 -v 0.2 -s 10000\n";
    std::cout << "  ./currency_simulator --model Vasicek --initial 0.05 --volatility 0.02\n";
    std::cout << "  ./currency_simulator --config configs/usd_rub.json\n";
}

struct CommandLineOptions {
    double initialPrice = 75.0;
    double drift = 0.05;
    double volatility = 0.2;
    double meanReversion = 1.0;
    double longTermMean = 0.05;
    std::string modelType = "GBM";
    int numSimulations = 10000;
    int numSteps = 252;
    double timeStep = 1.0 / 252.0;
    std::string outputPrefix = "simulation";
    unsigned int seed = 12345;
    int numThreads = 0;
    bool savePaths = false;
    int batchSize = 1000;
    bool riskAnalysis = false;
    std::string configFile;
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
        else if (arg == "-m" || arg == "--model") {
            if (i + 1 < argc) options.modelType = argv[++i];
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
        else if (arg == "--config") {
            if (i + 1 < argc) options.configFile = argv[++i];
        }
        else if (arg == "--save-paths") {
            options.savePaths = true;
        }
        else if (arg == "--batch-size") {
            if (i + 1 < argc) options.batchSize = std::stoi(argv[++i]);
        }
        else if (arg == "--risk-analysis") {
            options.riskAnalysis = true;
        }
    }
    
    return options;
}

std::shared_ptr<StochasticModel> createModel(const CommandLineOptions& options) {
    if (options.modelType == "GBM") {
        return std::make_shared<GBM>(options.initialPrice, options.drift, options.volatility);
    }
    else if (options.modelType == "Vasicek") {
        return std::make_shared<Vasicek>(options.initialPrice, options.meanReversion, 
                                        options.longTermMean, options.volatility);
    }
    else if (options.modelType == "HullWhite") {
        return std::make_shared<HullWhite>(options.initialPrice, options.meanReversion, 
                                          options.volatility);
    }
    else {
        std::cerr << "Warning: Unknown model type '" << options.modelType 
                  << "'. Using GBM as default.\n";
        return std::make_shared<GBM>(options.initialPrice, options.drift, options.volatility);
    }
}

void loadConfigFromFile(CommandLineOptions& options, const std::string& filename) {
    JSONConfig config(filename);
    
    if (config.hasKey("model")) {
        options.modelType = config.getString("model", "GBM");
    }
    
    if (config.hasKey("initial_price")) {
        options.initialPrice = config.getDouble("initial_price", 75.0);
    }
    
    if (config.hasKey("drift")) {
        options.drift = config.getDouble("drift", 0.05);
    }
    
    if (config.hasKey("volatility")) {
        options.volatility = config.getDouble("volatility", 0.2);
    }
    
    if (config.hasKey("mean_reversion")) {
        options.meanReversion = config.getDouble("mean_reversion", 1.0);
    }
    
    if (config.hasKey("long_term_mean")) {
        options.longTermMean = config.getDouble("long_term_mean", 0.05);
    }
    
    if (config.hasKey("num_simulations")) {
        options.numSimulations = config.getInt("num_simulations", 10000);
    }
    
    if (config.hasKey("num_steps")) {
        options.numSteps = config.getInt("num_steps", 252);
    }
    
    if (config.hasKey("dt")) {
        options.timeStep = config.getDouble("dt", 1.0/252.0);
    }
    
    if (config.hasKey("seed")) {
        options.seed = config.getInt("seed", 12345);
    }
    
    if (config.hasKey("batch_size")) {
        options.batchSize = config.getInt("batch_size", 1000);
    }
}

void printParameters(const CommandLineOptions& options) {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n=== Simulation Parameters ===\n";
    std::cout << "Model: " << options.modelType << "\n";
    std::cout << "Initial Price: " << options.initialPrice << "\n";
    
    if (options.modelType == "GBM") {
        std::cout << "Drift (μ): " << options.drift << " (" << options.drift*100 << "%)\n";
        std::cout << "Volatility (σ): " << options.volatility 
                  << " (" << options.volatility*100 << "%)\n";
    }
    else if (options.modelType == "Vasicek" || options.modelType == "HullWhite") {
        std::cout << "Mean Reversion: " << options.meanReversion << "\n";
        std::cout << "Long Term Mean: " << options.longTermMean << "\n";
        std::cout << "Volatility: " << options.volatility << "\n";
    }
    
    std::cout << "Number of Simulations: " << options.numSimulations << "\n";
    std::cout << "Number of Steps: " << options.numSteps << "\n";
    std::cout << "Time Step: " << options.timeStep << " years\n";
    std::cout << "Time Horizon: " << options.numSteps * options.timeStep << " years\n";
    std::cout << "Random Seed: " << options.seed << "\n";
    
#ifdef _OPENMP
    if (options.numThreads > 0) {
        std::cout << "Threads: " << options.numThreads << "\n";
    } else {
        std::cout << "Threads: auto\n";
    }
#endif
}

int main(int argc, char* argv[]) {
    // Setup logger
    Logger::getInstance().setConsoleOutput(true);
    Logger::getInstance().setMinLevel(LogLevel::INFO);
    
    if (argc == 1) {
        std::cout << "No arguments provided. Using default parameters.\n";
        std::cout << "Use -h or --help for usage information.\n\n";
    }
    
    CommandLineOptions options = parseArguments(argc, argv);
    
    // Load configuration from file if specified
    if (!options.configFile.empty()) {
        std::cout << "Loading configuration from: " << options.configFile << "\n";
        loadConfigFromFile(options, options.configFile);
    }
    
    printParameters(options);
    
    try {
        // 1. Create model
        auto model = createModel(options);
        
        // 2. Create random number generator
        auto randomGen = std::make_shared<NormalGenerator>(0.0, 1.0, options.seed);
        
        // 3. Create simulator
        MonteCarloSimulator simulator(model, randomGen);
        
        if (options.numThreads > 0) {
            simulator.setNumThreads(options.numThreads);
        }
        
        // 4. Run simulation
        std::cout << "\nRunning Monte Carlo simulation...\n";
        Timer simulationTimer("Simulation");
        
        SimulationResult result;
        if (options.numSimulations > 10000 && options.batchSize > 0) {
            std::cout << "Using batch processing (batch size: " 
                      << options.batchSize << ")\n";
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
        
        simulationTimer.stop();
        
        // 5. Display results
        std::cout << "\n=== Simulation Results ===\n";
        std::cout << "Execution Time: " << result.executionTime << " seconds\n";
        std::cout << "Simulations per second: " 
                  << options.numSimulations / result.executionTime << "\n";
        
        result.print();
        
        // 6. Detailed risk analysis if requested
        if (options.riskAnalysis) {
            std::cout << "\n=== Detailed Risk Analysis ===\n";
            ResultAnalyzer analyzer(result.finalPrices);
            analyzer.printAnalysis();
        }
        
        // 7. Calculate additional metrics
        std::cout << "\n=== Additional Metrics ===\n";
        
        // Probability of price increase
        double target10 = options.initialPrice * 1.10;
        double target20 = options.initialPrice * 1.20;
        double targetDown10 = options.initialPrice * 0.90;
        
        int count10 = 0, count20 = 0, countDown10 = 0;
        for (double price : result.finalPrices) {
            if (price >= target10) count10++;
            if (price >= target20) count20++;
            if (price <= targetDown10) countDown10++;
        }
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Probability Analysis:\n";
        std::cout << "  Price >= " << target10 << " (+10%): " 
                  << (100.0 * count10 / options.numSimulations) << "%\n";
        std::cout << "  Price >= " << target20 << " (+20%): " 
                  << (100.0 * count20 / options.numSimulations) << "%\n";
        std::cout << "  Price <= " << targetDown10 << " (-10%): " 
                  << (100.0 * countDown10 / options.numSimulations) << "%\n";
        
        // 8. Save results to files
        std::cout << "\nSaving results to files...\n";
        
        std::string timestamp = std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        // Save final prices
        std::string pricesFile = options.outputPrefix + "_prices_" + timestamp + ".csv";
        CSVWriter::writeVector(result.finalPrices, pricesFile, "FinalPrice");
        std::cout << "Final prices saved to: " << pricesFile << "\n";
        
        // Save summary
        std::string summaryFile = options.outputPrefix + "_summary_" + timestamp + ".csv";
        CSVWriter::writeSummary(result.finalPrices, summaryFile);
        std::cout << "Summary saved to: " << summaryFile << "\n";
        
        // Save sample paths (first 100)
        if (options.savePaths && !result.paths.empty()) {
            std::string pathsFile = options.outputPrefix + "_paths_" + timestamp + ".csv";
            // Save only first 100 paths to avoid huge files
            size_t numPathsToSave = std::min(result.paths.size(), size_t(100));
            std::vector<std::vector<double>> samplePaths(
                result.paths.begin(), 
                result.paths.begin() + numPathsToSave
            );
            CSVWriter::writePaths(samplePaths, pathsFile);
            std::cout << "Sample paths saved to: " << pathsFile << "\n";
        }
        
        // Save risk metrics
        if (options.riskAnalysis) {
            std::string riskFile = options.outputPrefix + "_risk_" + timestamp + ".csv";
            ResultAnalyzer analyzer(result.finalPrices);
            auto report = analyzer.generateReport();
            
            std::vector<std::string> headers = {
                "Price_Mean", "Price_Median", "Price_StdDev", "Price_Min", "Price_Max",
                "Return_Mean", "Return_StdDev", "Return_Skewness", "Return_Kurtosis",
                "VaR_95", "CVaR_95", "Volatility", "Sharpe_Ratio", "Max_Drawdown"
            };
            
            // Flatten the report
            std::vector<std::vector<double>> flatReport;
            std::vector<double> row;
            for (const auto& section : report) {
                row.insert(row.end(), section.begin(), section.end());
            }
            flatReport.push_back(row);
            
            CSVWriter::write(flatReport, riskFile, headers);
            std::cout << "Risk metrics saved to: " << riskFile << "\n";
        }
        
        // 9. Generate configuration file for reproducibility
        std::string configOutputFile = options.outputPrefix + "_config_" + timestamp + ".json";
        JSONConfig outputConfig;
        outputConfig.setValue("model", options.modelType);
        outputConfig.setValue("initial_price", std::to_string(options.initialPrice));
        outputConfig.setValue("drift", std::to_string(options.drift));
        outputConfig.setValue("volatility", std::to_string(options.volatility));
        outputConfig.setValue("num_simulations", std::to_string(options.numSimulations));
        outputConfig.setValue("num_steps", std::to_string(options.numSteps));
        outputConfig.setValue("dt", std::to_string(options.timeStep));
        outputConfig.setValue("seed", std::to_string(options.seed));
        outputConfig.setValue("execution_time", std::to_string(result.executionTime));
        outputConfig.save(configOutputFile);
        std::cout << "Configuration saved to: " << configOutputFile << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        LOG_ERROR("Simulation failed: " + std::string(e.what()));
        return 1;
    }
    
    std::cout << "\n=== Simulation completed successfully! ===\n";
    LOG_INFO("Simulation completed successfully");
    
    std::cout << "\nNext steps:\n";
    std::cout << "1. Use Python scripts for visualization:\n";
    std::cout << "   python scripts/visualize.py --prices-file " << options.outputPrefix << "_prices_*.csv\n";
    std::cout << "2. Analyze risk metrics:\n";
    std::cout << "   python scripts/analyze_results.py --input " << options.outputPrefix << "_summary_*.csv\n";
    
    return 0;
}
