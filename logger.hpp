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
#include <regex>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class Logger {
public:
    enum Level  {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger& get_instance(const std::string& filedirectory = "", const std::string& filename = "", size_t max_size = 5 * 1024 * 1024, size_t max_num = 10) {
        static Logger instance(filedirectory, filename, max_size, max_num);
        return instance;
    }

    template<typename ...Args>
    void debug(const char* file, int line, const std::string& format, Args&&... args) {
        log(Debug, file, line, format, args...);
    }
    template<typename ...Args>
    void info(const char* file, int line, const std::string& format, Args&&... args) {
        log(Info, file, line, format, args...);
    }
    template<typename ...Args>
    void warning(const char* file, int line, const std::string& format, Args&&... args) {
        log(Warning, file, line, format, args...);
    }
    template<typename ...Args>
    void error(const char* file, int line, const std::string& format, Args&&... args) {
        log(Error, file, line, format, args...);
    }

    void set_level(Level level) {
        level_ = level;
    }

private:
    Logger(const std::string& filedirectory, const std::string& filename, size_t max_size, size_t max_num) : max_size_(max_size), max_num_(max_num), stop_(false) {

#ifdef _WIN32
        pid_ = GetCurrentProcessId();
#else
        pid_ = getpid();
#endif

        if (filedirectory.empty()) {
            file_directory_ = default_file_directory_;
        }
        else{
            file_directory_ = filedirectory;
        }

        if (filename.empty()) {
            filename_ = default_filename_;
        }
        else {
            filename_ = filename;
        }

        if(!std::filesystem::exists(file_directory_)) {
            std::error_code ec;
            auto bRet = std::filesystem::create_directories(file_directory_, ec);
            if(!bRet) {
                std::cout << "[Error] Failed to create the log file directory(" << file_directory_ <<"). error message: " << ec.message() << std::endl;
                return;
            }
        }

        file_directory_ = std::filesystem::absolute(file_directory_).string();

        auto last_filename = find_last_log_file(filename_);
        if(last_filename.empty()) {
            filenpath_ = std::filesystem::path(file_directory_).append(get_filename_with_timpstamp()).string();
        }
        else {
            filenpath_ = std::filesystem::path(file_directory_).append(last_filename).string();
        }

        file_stream_ = std::make_unique<std::ofstream>(filenpath_, std::ios::out | std::ios::app);
        if (!file_stream_->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filenpath_.string());
        }

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
    void log(Level level, const char* file, int line, const std::string& format, Args&&... args) {
        if (level < level_) return;

        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
        std::tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_c);
#else
        localtime_r(&now_c, &now_tm);
#endif
        std::stringstream msg;
        msg << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << now_ms
            << " [" << pid_ << ":" << std::this_thread::get_id() << "] [" << level_to_string(level) << "] [" << file << ":" << line << "] ";

        log_impl(msg, format, std::forward<Args>(args)...);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(msg.str());
            cv_.notify_one();
        }
    }

    template<typename T, typename... Args>
    void log_impl(std::ostream& os, const std::string& format, T&& arg, Args&&... args) {
        size_t placeholder_pos = format.find("{}");
        if (placeholder_pos != std::string::npos) {
            os << format.substr(0, placeholder_pos);
            os << std::forward<T>(arg);
            log_impl(os, format.substr(placeholder_pos + 2), std::forward<Args>(args)...);
        } else {
            os << format;
        }
    }

    void log_impl(std::ostream& os, const std::string& format) {
        os << format;
    }

    constexpr const std::string& level_to_string(Level level) {
        return levels_[level];
    }

    std::string find_last_log_file(const std::string& filename) {
        std::filesystem::path filenamepath(filename);
        std::string str_regex;
        str_regex += "^";
        str_regex += filenamepath.stem().string();
        str_regex += "_\\d{8}_\\d{6}_\\d{3}";
        str_regex += filenamepath.extension().string();
        str_regex += "$";
        std::regex log_file_regex(str_regex);
        std::vector<std::string> match_list;
        for (const auto& entry : std::filesystem::directory_iterator(file_directory_)) {
            if (entry.is_regular_file() && std::regex_match(entry.path().filename().string(), log_file_regex)) {
                match_list.emplace_back(entry.path().filename().string());
            }
        }

        if(match_list.empty()) {
            return std::string();
        }

        std::sort(match_list.begin(), match_list.end());

        if (match_list.size() > max_num_) {
            // If the limit is exceeded, delete the oldest log file
            std::filesystem::remove(std::filesystem::path(file_directory_).append(match_list.front()));
        }

        return match_list.back();
    }

    inline std::string get_filename_with_timpstamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_c);
#else
        localtime_r(&now_c, &now_tm);
#endif
        std::stringstream ss;
        std::filesystem::path tmp_path(filename_);
        ss << tmp_path.stem().string() << std::put_time(&now_tm, "_%Y%m%d_%H%M%S_") << std::setfill('0') << std::setw(3) << now_ms.count() << tmp_path.extension().string();
        return ss.str();
    }

    void write_to_file() {
        while (true) { // loop until stop
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || stop_; }); // wait until the queue is not empty or stop is true
            if (stop_ && queue_.empty()) break; // if stop is true and the queue is empty, break the loop
            auto msg = queue_.front(); // get the front message from the queue
            queue_.pop(); // pop the message from the queue
            lock.unlock(); // unlock the mutex

            check_file_size(msg.size()); // check if the current file size exceeds the limit
            *file_stream_ << msg << std::endl; // write the message to the file
        }
    }

    void check_file_size(size_t next_msg_size) {
        if ((std::filesystem::file_size(filenpath_) + next_msg_size) > max_size_) { // if the current file size is larger than the limit
            file_stream_->close(); // close the current file stream
            filenpath_ = std::filesystem::path(file_directory_).append(get_filename_with_timpstamp()).string(); // get a new file name
            file_stream_->open(filenpath_, std::ios::out | std::ios::app); // open a new file stream
            if (!file_stream_->is_open()) { // check if the new file stream is opened successfully
                throw std::runtime_error("Failed to open log file: " + filenpath_.string());
            }
        }
    }

    const std::array<std::string, 4> levels_ = {"DEBUG", "INFO", "WARNING", "ERROR"};

    unsigned long pid_ = 0;                                     // the current process id
    const std::string default_file_directory_ = "logs";         // the default file directory
    const std::string default_filename_ = "log.txt";            // the default file name
    std::string file_directory_;                                // the current file directory
    std::string filename_;                                      // the current file name
    Level level_ = Level::Debug;                                // the default log level
    size_t max_size_;                                           // the maximum file size in bytes
    size_t max_num_;                                            // the maximum number of files
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
