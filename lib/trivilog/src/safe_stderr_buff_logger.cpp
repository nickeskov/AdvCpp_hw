#include "trivilog/safe_stderr_buff_logger.h"

namespace trivilog {

void SafeStderrBuffLogger::log_to_ostream(std::string_view log_level_name, std::string_view msg) {
    std::lock_guard<std::mutex> guard(mutex_);
    BaseLogger::log_to_ostream(log_level_name, msg);
}

}
