# MiniLogger

### 特性
- 区分日志等级
- 包含时间信息，并精确到毫秒级别
- 包含文件名和行号
- 包含进程PID

### 使用方法

使用示例
```` cpp
int main() 
{
    Logger logger("example.log", Logger::DEBUG);

    LOG_DEBUG(logger, "Debug message");
    LOG_INFO(logger, "Info message");
    LOG_WARNING(logger, "Warning message");
    LOG_ERROR(logger, "Error message");

    return 0;
}
````


日志文件内容
````
2023-03-18 15:29:38.950 [9384] [DEBUG] ..\LoggerTest\main.cpp:8 Debug message 
2023-03-18 15:29:38.950 [9384] [INFO] ..\LoggerTest\main.cpp:9 Info message 
2023-03-18 15:29:38.951 [9384] [WARNING] ..\LoggerTest\main.cpp:10 Warning message 
2023-03-18 15:29:38.951 [9384] [ERROR] ..\LoggerTest\main.cpp:11 Error message 
````