#include "logger.hpp"

#ifdef _WIN32
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

int main()
{
    // Get the logger instance
    auto& logger = Logger::get_instance("Logs", "test.log", 2*1024*1024, 5);

    // Set the log level to debug
    logger.set_level(Logger::Debug);

    LOG_DEBUG("This is a message");
    LOG_DEBUG("This is a message with one argument, {}","logger test.");
    LOG_WARNING("This is a message with multiple arguments, arg1={}, arg2={}, arg3={}", 123, 0.618f, "hello!");

    return 0;
}

