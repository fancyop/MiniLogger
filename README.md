# MiniLogger

### 日志类特性
- 支持区分日志等级
- 包含日期时间（毫秒级别）信息
- 包含文件名和行号
- 包含进程PID和线程TID
- 支持异步文件写入
- 支持指定文件大小分片
- 支持指定文件个数限制
- 支持值按占位符插入的格式化

### 使用方法

使用示例

```` cpp
#include "logger.hpp"

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
````

日志文件命名规则

````
.
├── Logs
│   ├── test_20230421_182939_461.log
│   ├── test_20230421_182942_674.log
│   └── test_20230421_182953_423.log
````

日志文件内容

````

2023-05-30 17:26:32.386 [28924:21076] [DEBUG] [X:\MiniLogger\main.cpp:15] This is a message
2023-05-30 17:26:32.388 [28924:21076] [DEBUG] [X:\MiniLogger\main.cpp:16] This is a message with one argument, logger test.
2023-05-30 17:26:32.388 [28924:21076] [WARNING] [X:\MiniLogger\main.cpp:17] This is a message with multiple arguments, arg1=123, arg2=0.618, arg3=hello!

````