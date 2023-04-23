# MiniLogger

### 特性
- 区分日志等级
- 包含时间信息，并精确到毫秒级别
- 包含文件名和行号
- 包含进程PID和线程TID
- 支持文件按指定大小分片
- 独立线程进行写文件操作
- 文件名包含时间戳

### 使用方法

使用示例
```` cpp
#include "logger.hpp"

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
2023-04-18 09:17:00.128 [29504:24792] [DEBUG] [..\MiniLogger\main.cpp:13] This is a message: 
2023-04-18 09:17:00.130 [29504:24792] [WARNING] [..\MiniLogger\main.cpp:14] This is a message with multiple arguments: 1 2 3

````