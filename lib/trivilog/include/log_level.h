#ifndef HW_TRIVILOG_LOG_LEVEL_H
#define HW_TRIVILOG_LOG_LEVEL_H

namespace trivilog {

enum class log_level : unsigned int {
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    INFO = 4,
    DEBUG = 5,
    TRACE = 6
};

}

#endif //HW_TRIVILOG_LOG_LEVEL_H
