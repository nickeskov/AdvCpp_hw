#ifndef HW_LIB_TRIVILOG_TRIVILOG_SAFE_STDERR_BUFF_LOGGER_H
#define HW_LIB_TRIVILOG_TRIVILOG_SAFE_STDERR_BUFF_LOGGER_H

#include <mutex>

#include "trivilog/stderr_buff_logger.h"

namespace trivilog {

class SafeStderrBuffLogger : public StderrBuffLogger {
  public:

    SafeStderrBuffLogger(const SafeStderrBuffLogger &) = delete;

    SafeStderrBuffLogger &operator=(const SafeStderrBuffLogger &) = delete;

    ~SafeStderrBuffLogger() noexcept override = default;

  protected:
    void log_to_ostream(std::string_view log_level_name, std::string_view msg) override;

  private:
    std::mutex mutex_;
};

}

#endif //HW_LIB_TRIVILOG_TRIVILOG_SAFE_STDERR_BUFF_LOGGER_H
