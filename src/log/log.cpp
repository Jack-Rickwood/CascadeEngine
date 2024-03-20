#include "log.h"

namespace cscd {

void Logger::log(LogLevel level, std::string message) {
    std::cout << LogLevelInfoString[level] << " " << message << std::endl;
    if (level == LOG_LEVEL_CRITICAL) {
        exit(-1);
    }
}

}