#ifndef HW_TRIVILOG_LOG_LEVEL_H
#define HW_TRIVILOG_LOG_LEVEL_H

namespace trivilog {

enum class log_level : unsigned int {
    FATAL = 0,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

}

#endif //HW_TRIVILOG_LOG_LEVEL_H
