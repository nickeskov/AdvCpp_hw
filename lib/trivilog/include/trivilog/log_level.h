#ifndef TRIVILOG_TRIVILOG_LOG_LEVEL_H
#define TRIVILOG_TRIVILOG_LOG_LEVEL_H

namespace trivilog {

enum class log_level : unsigned int {
    FATAL = 0,
    CRIT,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

}

#endif //TRIVILOG_TRIVILOG_LOG_LEVEL_H
