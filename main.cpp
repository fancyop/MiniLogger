#include "logger.hpp"

#ifdef _WIN32
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

int main()
{
    // Get the logger instance
    auto& logger = Logger::get_instance("Logs", "test.log", 2*1000, 5);

    // Set the log level to debug
    logger.set_level(Logger::Debug);

    LOG_DEBUG("This is a message: ");
    LOG_WARNING("This is a message with multiple arguments: ", 1, " ", 2.0, " ", '3');

    return 0;
}

