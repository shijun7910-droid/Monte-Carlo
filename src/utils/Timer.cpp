#include "utils/Timer.h"
#include <iomanip>
#include <sstream>

Timer::Timer(const std::string& name) : name_(name) {
    start();
}

Timer::~Timer() {
    if (endTime_ == std::chrono::high_resolution_clock::time_point{}) {
        stop();
    }
    print();
}

void Timer::start() {
    startTime_ = std::chrono::high_resolution_clock::now();
    endTime_ = std::chrono::high_resolution_clock::time_point{};
}

void Timer::stop() {
    endTime_ = std::chrono::high_resolution_clock::now();
}

double Timer::elapsedSeconds() const {
    if (endTime_ == std::chrono::high_resolution_clock::time_point{}) {
        auto current = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(current - startTime_);
        return duration.count();
    }
    auto duration = std::chrono::duration<double>(endTime_ - startTime_);
    return duration.count();
}

double Timer::elapsedMilliseconds() const {
    if (endTime_ == std::chrono::high_resolution_clock::time_point{}) {
        auto current = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(current - startTime_);
        return duration.count();
    }
    auto duration = std::chrono::duration<double, std::milli>(endTime_ - startTime_);
    return duration.count();
}

double Timer::elapsedMicroseconds() const {
    if (endTime_ == std::chrono::high_resolution_clock::time_point{}) {
        auto current = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::micro>(current - startTime_);
        return duration.count();
    }
    auto duration = std::chrono::duration<double, std::micro>(endTime_ - startTime_);
    return duration.count();
}

void Timer::print() const {
    double seconds = elapsedSeconds();
    
    std::ostringstream oss;
    oss << name_ << ": ";
    
    if (seconds < 0.000001) {
        oss << elapsedMicroseconds() << " μs";
    } else if (seconds < 0.001) {
        oss << elapsedMilliseconds() << " ms";
    } else if (seconds < 1.0) {
        oss << std::fixed << std::setprecision(3) << seconds * 1000.0 << " ms";
    } else if (seconds < 60.0) {
        oss << std::fixed << std::setprecision(3) << seconds << " s";
    } else if (seconds < 3600.0) {
        int minutes = static_cast<int>(seconds / 60);
        double remaining = seconds - minutes * 60.0;
        oss << minutes << "m " << std::fixed << std::setprecision(1) << remaining << "s";
    } else {
        int hours = static_cast<int>(seconds / 3600);
        int minutes = static_cast<int>((seconds - hours * 3600.0) / 60);
        double remaining = seconds - hours * 3600.0 - minutes * 60.0;
        oss << hours << "h " << minutes << "m " << std::fixed << std::setprecision(1) << remaining << "s";
    }
    
    std::cout << oss.str() << std::endl;
}

void Timer::print(const std::string& message) const {
    std::cout << message << ": " << std::fixed << std::setprecision(6) 
              << elapsedSeconds() << " seconds" << std::endl;
}

Timer Timer::startNew(const std::string& name) {
    return Timer(name);
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(const std::string& name) : timer_(name) {}

ScopedTimer::~ScopedTimer() {
    timer_.stop();
    timer_.print();
}

double ScopedTimer::elapsed() const {
    return timer_.elapsedSeconds();
}

// Benchmark functions
void benchmarkFunction(const std::function<void()>& func, 
                      const std::string& name, 
                      int iterations) {
    Timer timer(name + " (" + std::to_string(iterations) + " iterations)");
    
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    
    timer.stop();
    std::cout << "Average per iteration: " 
              << timer.elapsedSeconds() / iterations * 1000.0 
              << " ms" << std::endl;
}

// Performance counter
PerformanceCounter::PerformanceCounter(const std::string& name) 
    : name_(name), count_(0), totalTime_(0.0) {}

void PerformanceCounter::start() {
    startTime_ = std::chrono::high_resolution_clock::now();
}

void PerformanceCounter::stop() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(endTime - startTime_);
    totalTime_ += duration.count();
    count_++;
}

void PerformanceCounter::reset() {
    count_ = 0;
    totalTime_ = 0.0;
}

double PerformanceCounter::averageTime() const {
    return count_ > 0 ? totalTime_ / count_ : 0.0;
}

double PerformanceCounter::totalTime() const {
    return totalTime_;
}

int PerformanceCounter::count() const {
    return count_;
}

void PerformanceCounter::print() const {
    std::cout << name_ << ": " << count_ << " calls, "
              << "total: " << totalTime_ << " s, "
              << "average: " << averageTime() * 1000.0 << " ms/call" 
              << std::endl;
}
