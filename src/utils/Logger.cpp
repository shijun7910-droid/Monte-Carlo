#include "utils/Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

// Initialize static instance
Logger* Logger::instance_ = nullptr;

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm* localTime = std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (logFile_.is_open()) {
        logFile_.close();
    }
    
    logFile_.open(filename, std::ios::app);
    if (!logFile_.is_open()) {
        std::cerr << "Error: Could not open log file " << filename << std::endl;
    } else {
        info("Log file opened: " + filename);
    }
}

void Logger::setMinLevel(LogLevel level) {
    minLevel_ = level;
}

void Logger::setConsoleOutput(bool enable) {
    consoleOutput_ = enable;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string timestamp = getCurrentTime();
    std::string levelStr = levelToString(level);
    std::string logEntry = timestamp + " [" + levelStr + "] " + message;
    
    // Console output with colors (ANSI escape codes)
    if (consoleOutput_) {
        std::ostream& output = (level >= LogLevel::WARNING) ? std::cerr : std::cout;
        
        // Set color based on log level
        switch (level) {
            case LogLevel::DEBUG:
                output << "\033[90m";  // Gray
                break;
            case LogLevel::INFO:
                output << "\033[37m";  // White
                break;
            case LogLevel::WARNING:
                output << "\033[33m";  // Yellow
                break;
            case LogLevel::ERROR:
                output << "\033[31m";  // Red
                break;
            case LogLevel::CRITICAL:
                output << "\033[1;31m";  // Bold Red
                break;
        }
        
        output << logEntry << "\033[0m" << std::endl;
    }
    
    // File output (no colors)
    if (logFile_.is_open()) {
        logFile_ << logEntry << std::endl;
        logFile_.flush();  // Ensure immediate writing
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        info("Closing log file");
        logFile_.close();
    }
}

// Additional utility functions
void Logger::logWithLocation(LogLevel level, 
                            const std::string& message,
                            const std::string& file,
                            int line) {
    std::string fullMessage = message + " [" + file + ":" + std::to_string(line) + "]";
    log(level, fullMessage);
}

void Logger::logProgress(double progress, const std::string& task) {
    if (!consoleOutput_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    int barWidth = 50;
    std::cout << "\r" << task << " [";
    
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    
    std::cout << "] " << std::fixed << std::setprecision(1) 
              << (progress * 100.0) << "%" << std::flush;
    
    if (progress >= 1.0) {
        std::cout << std::endl;
    }
}

void Logger::logTable(const std::vector<std::string>& headers,
                     const std::vector<std::vector<std::string>>& rows) {
    if (!consoleOutput_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Calculate column widths
    std::vector<size_t> colWidths(headers.size(), 0);
    
    for (size_t i = 0; i < headers.size(); ++i) {
        colWidths[i] = std::max(colWidths[i], headers[i].length());
    }
    
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            colWidths[i] = std::max(colWidths[i], row[i].length());
        }
    }
    
    // Print header
    std::cout << "\n";
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::left << std::setw(colWidths[i] + 2) << headers[i];
    }
    std::cout << "\n";
    
    // Print separator
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::string(colWidths[i] + 2, '-');
    }
    std::cout << "\n";
    
    // Print rows
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << std::left << std::setw(colWidths[i] + 2) << row[i];
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}
