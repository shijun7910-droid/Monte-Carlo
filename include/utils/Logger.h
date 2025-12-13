#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
private:
    static Logger* instance_;
    std::ofstream logFile_;
    LogLevel minLevel_;
    bool consoleOutput_;
    std::mutex mutex_;
    
    Logger() : minLevel_(LogLevel::INFO), consoleOutput_(true) {}
    
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }
    
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logFile_.is_open()) {
            logFile_.close();
        }
        logFile_.open(filename, std::ios::app);
    }
    
    void setMinLevel(LogLevel level) {
        minLevel_ = level;
    }
    
    void setConsoleOutput(bool enable) {
        consoleOutput_ = enable;
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < minLevel_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::string logEntry = getCurrentTime() + " [" + levelToString(level) + "] " + message;
        
        if (consoleOutput_) {
            if (level >= LogLevel::WARNING) {
                std::cerr << logEntry << std::endl;
            } else {
                std::cout << logEntry << std::endl;
            }
        }
        
        if (logFile_.is_open()) {
            logFile_ << logEntry << std::endl;
        }
    }
    
    void debug(const std::string& message) {
        log(LogLevel::DEBUG, message);
    }
    
    void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }
    
    void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }
    
    void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }
    
    void critical(const std::string& message) {
        log(LogLevel::CRITICAL, message);
    }
    
    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) Logger::getInstance().critical(msg)

#endif // LOGGER_H
