#include "logger.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址1

int main() {
    // 获取单例对象
    Logger::get_instance("example.log");

    // 设置日志级别
    Logger::get_instance().set_level(Logger::Level::WARNING);

    // 输出日志
    LOG_DEBUG("debug message");
    LOG_INFO("info message");
    LOG_WARNING("warning message");
    LOG_ERROR("error message");

    return 0;
}


