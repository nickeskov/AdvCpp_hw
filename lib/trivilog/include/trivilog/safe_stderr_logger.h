#ifndef TRIVILOG_TRIVILOG_SAFE_STDERR_LOGGER_H
#define TRIVILOG_TRIVILOG_SAFE_STDERR_LOGGER_H

#include <mutex>

#include "trivilog/stderr_logger.h"

namespace trivilog {

class SafeStderrLogger : public StderrLogger {
  public:

    SafeStderrLogger(const SafeStderrLogger &) = delete;

    SafeStderrLogger &operator=(const SafeStderrLogger &) = delete;

    ~SafeStderrLogger() noexcept override = default;

  protected:
    void log_to_ostream(std::string_view log_level_name, std::string_view msg) override;

  private:
    std::mutex mutex_;
};


}

#endif //HW_LIB_TRIVILOG_TRIVILOG_SAFE_STDERR_LOGGER_H
