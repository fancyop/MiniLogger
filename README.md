# MiniLogger

### 特性
- 区分日志等级
- 包含时间信息，并精确到毫秒级别
- 包含文件名和行号
- 包含线程程PID
- 支持文件按指定大小分片
- 独立线程进行写文件操作

### 使用方法

使用示例
```` cpp
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
````


日志文件内容
````
2023-04-07 19:06:11.277 [5068] [DEBUG] [..\MiniLogger\main.cpp:13] This is a message: 
2023-04-07 19:06:11.279 [5068] [WARNING] [..\MiniLogger\main.cpp:14] This is a message with multiple arguments: 1 2 3

````