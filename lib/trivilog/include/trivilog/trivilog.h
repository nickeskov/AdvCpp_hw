#ifndef TRIVILOG_TRIVILOG_TRIVILOG_H
#define TRIVILOG_TRIVILOG_TRIVILOG_H

#include <memory>
#include <type_traits>

#include "trivilog/log_level.h"
#include "trivilog/base_logger.h"

#include "trivilog/stdout_logger.h"
#include "trivilog/stderr_logger.h"
#include "trivilog/stderr_buff_logger.h"
#include "trivilog/file_logger.h"
#include "trivilog/global_logger.h"

namespace trivilog {

template<typename T, typename ...Args,
        std::enable_if_t<std::is_base_of_v<BaseLogger, T>, T> * = nullptr>
std::unique_ptr<T> create_logger(Args &&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

}

#endif //TRIVILOG_TRIVILOG_TRIVILOG_H
