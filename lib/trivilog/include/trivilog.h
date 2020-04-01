#ifndef HW_TRIVILOG_TRIVILOG_H
#define HW_TRIVILOG_TRIVILOG_H

#include "log_level.h"

#include "base_logger.h"

#include "trivilog_errors.h"
#include "stdout_logger.h"
#include "stderr_logger.h"
#include "stderr_buff_logger.h"
#include "file_logger.h"
#include "global_logger.h"

#include <memory>
#include <type_traits>

namespace trivilog {

template <typename T, typename ...Args,
        std::enable_if_t<std::is_base_of_v<BaseLogger, T>, T>* = nullptr>
std::unique_ptr<T> create_logger(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

}

#endif //HW_TRIVILOG_TRIVILOG_H
