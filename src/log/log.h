#include <string>
#include <iostream>
#include <vector>

namespace cscd {

class Logger {
public:
    enum LogLevel {
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_CRITICAL
    };

    std::vector<std::string> LogLevelInfoString = {
        "[INFO]",
        "[WARNING]",
        "[ERROR]",
        "[CRITICAL]"
    };

    void log(LogLevel level, std::string message);
};

}