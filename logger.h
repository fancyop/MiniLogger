#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>

class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    Logger(const std::string& filename) {
        file_stream_.open(filename, std::ios::out | std::ios::app);
    }

    ~Logger() {
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    }

    template<typename ...Args>
    void log(Level level, const char* file, int line, Args... args) {
        if (level < level_) return;

        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

        file_stream_ << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S.");
        file_stream_ << std::setfill('0') << std::setw(3) << now_ms << " ";
        file_stream_ << "[" << std::this_thread::get_id() << "] ";
        file_stream_ << "[" << level_to_string(level) << "] ";
        file_stream_ << file << ":" << line << " ";
        log_impl(args...);
        file_stream_ << std::endl;
    }

    template<typename ...Args>
    void debug(const char* file, int line, Args... args) {
        log(DEBUG, file, line, args...);
    }

    template<typename ...Args>
    void info(const char* file, int line, Args... args) {
        log(INFO, file, line, args...);
    }

    template<typename ...Args>
    void warning(const char* file, int line, Args... args) {
        log(WARNING, file, line, args...);
    }

    template<typename ...Args>
    void error(const char* file, int line, Args... args) {
        log(ERROR, file, line, args...);
    }

    void set_level(Level level) {
        level_ = level;
    }

private:
    std::ofstream file_stream_;
    static Level level_; // 将level_变量声明为静态变量
    std::mutex mutex_;

    std::string level_to_string(Level level) {
        switch (level) {
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARNING:
                return "WARNING";
            case ERROR:
                return "ERROR";
            default:
                return "";
        }
    }

    void log_impl() {}

    template<typename T, typename ...Args>
    void log_impl(T&& arg, Args&&... args) {
        file_stream_ << std::forward<T>(arg) << " ";
        log_impl(std::forward<Args>(args)...);
    }
};

Logger::Level Logger::level_ = Logger::DEBUG;

#define LOG_DEBUG(logger, ...) \
    logger.debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(logger, ...) \
    logger.info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(logger, ...) \
    logger.warning(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(logger, ...) \
    logger.error(__FILE__, __LINE__, __VA_ARGS__)
