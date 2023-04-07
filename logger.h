#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <memory>
#include <filesystem>
#include <array>
#include <queue>

class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& get_instance(const std::string& filedirectory = "", const std::string& filename = "", size_t max_size = 5 * 1024 * 1024) {
        static Logger instance(filedirectory, filename, max_size);
        return instance;
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
    Logger(const std::string& filedirectory, const std::string& filename, size_t max_size) : max_size_(max_size), stop_(false) {


        std::string file_directory;
        if (filedirectory.empty()) {
            std::filesystem::create_directories(default_filedirectory_);
            file_directory = default_filedirectory_;
        }
        else {
            if(std::filesystem::exists(filedirectory)) {
                file_directory = filedirectory;
            }
            else {
                std::filesystem::create_directories(filedirectory);
                if(std::filesystem::exists(filedirectory)) {
                    file_directory = filedirectory;
                }
                else {
                    std::filesystem::create_directories(default_filedirectory_);
                    file_directory = default_filedirectory_;
                }
            }
        }

        if (filename.empty()) {
            filenpath_ = std::filesystem::absolute(file_directory).append(default_filename_).string();
        } else {
            filenpath_ = std::filesystem::absolute(file_directory).append(filename).string();
        }

        file_stream_ = std::make_unique<std::ofstream>(filenpath_, std::ios::out | std::ios::app);
        if (!file_stream_->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filenpath_.string());
        }
        file_stream_->seekp(0, std::ios::end);
        // start a new thread to write messages to file
        writer_thread_ = std::make_unique<std::thread>(&Logger::write_to_file, this);
    }

    ~Logger() {
        // stop the writer thread and join it
        stop_ = true;
        cv_.notify_one();
        writer_thread_->join();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<typename ...Args>
    void log(Level level, const char* file, int line, Args&&... args) {
        if (level < level_) return;

        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

        std::stringstream msg;
        msg << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << now_ms
            << " [" << std::this_thread::get_id() << "] [" << level_to_string(level) << "] [" << file << ":" << line << "] ";

        log_impl(msg, args...);
    }

    template<typename T>
    void log_impl(std::stringstream& msg, T&& arg) {
        msg << arg; // append the last argument to the message
        // push the message to the queue and notify the writer thread
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(msg.str());
            cv_.notify_one();
        }
    }

    template<typename T, typename ...Args>
    void log_impl(std::stringstream& msg, T&& arg, Args&&... args) {
        msg << arg; // append the current argument to the message
        log_impl(msg, args...); // recursively call log_impl with the remaining arguments
    }

    const char* level_to_string(Level level) {
        static const char* levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
        return levels[level];
    }

    void write_to_file() {
        while (true) { // loop until stop
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || stop_; }); // wait until the queue is not empty or stop is true
            if (stop_ && queue_.empty()) break; // if stop is true and the queue is empty, break the loop
            auto msg = queue_.front(); // get the front message from the queue
            queue_.pop(); // pop the message from the queue
            lock.unlock(); // unlock the mutex

            check_file_size(); // check if the current file size exceeds the limit
            *file_stream_ << msg << std::endl; // write the message to the file
        }
    }

    void check_file_size() {
        if (file_stream_->tellp() > max_size_) { // if the current file size is larger than the limit
            file_stream_->close(); // close the current file stream
            filenpath_ = get_next_filename(); // get a new file name
            file_stream_->open(filenpath_, std::ios::out | std::ios::app); // open a new file stream
            if (!file_stream_->is_open()) { // check if the new file stream is opened successfully
                throw std::runtime_error("Failed to open log file: " + filenpath_.string());
            }
        }
    }

    std::filesystem::path get_next_filename() {
        int i = 1; // start from 1
        while (true) { // loop until find a valid file name
            auto new_filepath = filenpath_.parent_path().append(filenpath_.stem().string() + "_" + std::to_string(i++) + filenpath_.extension().string()); // append a number to the original file name
            if (!std::filesystem::exists(new_filepath)) { // check if the new file name does not exist
                return new_filepath; // return the new file name
            }
        }
    }

    const std::string default_filedirectory_ = "logs";          // the default file directory
    const std::string default_filename_ = "log.txt";            // the default file name
    Level level_ = Level::DEBUG;                                // the default log level
    size_t max_size_;                                           // the maximum file size in bytes
    std::filesystem::path filenpath_;                           // the current file path
    std::unique_ptr<std::ofstream> file_stream_;                // the current file stream
    std::mutex mutex_;                                          // the mutex for thread-safety
    std::condition_variable cv_;                                // the condition variable for synchronization
    std::queue<std::string> queue_;                             // the queue for storing messages
    std::unique_ptr<std::thread> writer_thread_;                // the writer thread for writing messages to file
    bool stop_;                                                 // the flag for stopping the writer thread
};

// define a macro to simplify the logging statements
#define LOG_DEBUG(...) \
    Logger::get_instance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) \
    Logger::get_instance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) \
    Logger::get_instance().warning(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) \
    Logger::get_instance().error(__FILE__, __LINE__, __VA_ARGS__)
