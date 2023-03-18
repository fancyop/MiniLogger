#include "logger.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址1


int main() {
    Logger logger("example.log");

    LOG_DEBUG(logger, "Debug message");
    LOG_INFO(logger, "Info message");
    LOG_WARNING(logger, "Warning message");
    LOG_ERROR(logger, "Error message");

    return 0;
}

