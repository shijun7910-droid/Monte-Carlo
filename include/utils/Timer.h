#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <string>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point endTime_;
    std::string name_;
    
public:
    Timer(const std::string& name = "Timer") : name_(name) {
        start();
    }
    
    ~Timer() {
        stop();
        print();
    }
    
    void start() {
        startTime_ = std::chrono::high_resolution_clock::now();
    }
    
    void stop() {
        endTime_ = std::chrono::high_resolution_clock::now();
    }
    
    double elapsedSeconds() const {
        auto duration = std::chrono::duration<double>(endTime_ - startTime_);
        return duration.count();
    }
    
    double elapsedMilliseconds() const {
        auto duration = std::chrono::duration<double, std::milli>(endTime_ - startTime_);
        return duration.count();
    }
    
    double elapsedMicroseconds() const {
        auto duration = std::chrono::duration<double, std::micro>(endTime_ - startTime_);
        return duration.count();
    }
    
    void print() const {
        double seconds = elapsedSeconds();
        if (seconds < 0.001) {
            std::cout << name_ << ": " << elapsedMicroseconds() << " μs\n";
        } else if (seconds < 1.0) {
            std::cout << name_ << ": " << elapsedMilliseconds() << " ms\n";
        } else {
            std::cout << name_ << ": " << seconds << " s\n";
        }
    }
    
    void print(const std::string& message) const {
        std::cout << message << ": " << elapsedSeconds() << " seconds\n";
    }
    
    static Timer startNew(const std::string& name = "Timer") {
        return Timer(name);
    }
};

class ScopedTimer {
private:
    Timer timer_;
    
public:
    ScopedTimer(const std::string& name = "ScopedTimer") : timer_(name) {}
    
    ~ScopedTimer() {
        timer_.stop();
        timer_.print();
    }
    
    double elapsed() const {
        return timer_.elapsedSeconds();
    }
};

#endif // TIMER_H
