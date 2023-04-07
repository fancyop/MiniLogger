#include "logger.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址1

int main()
{
    // Get the logger instance
    auto& logger = Logger::get_instance("mylogs", "wwww.log", 10*1024*1024);

    // Set the log level to debug
    logger.set_level(Logger::DEBUG);

    LOG_DEBUG("This is a message: ");
    LOG_WARNING("This is a message with multiple arguments: ", 1, " ", 2.0, " ", '3');

    return 0;
}

