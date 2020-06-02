#ifndef TRIVILOG_TRIVILOG_SAFE_STDOUT_LOGGER_H
#define TRIVILOG_TRIVILOG_SAFE_STDOUT_LOGGER_H

#include <mutex>

#include "trivilog/stdout_logger.h"

namespace trivilog {

class SafeStdoutLogger : public StdoutLogger {
  public:

    SafeStdoutLogger(const SafeStdoutLogger &) = delete;

    SafeStdoutLogger &operator=(const SafeStdoutLogger &) = delete;

    ~SafeStdoutLogger() noexcept override = default;

  protected:
    void log_to_ostream(std::string_view log_level_name, std::string_view msg) override;

  private:
    std::mutex mutex_;
};

}
#endif //TRIVILOG_TRIVILOG_SAFE_STDOUT_LOGGER_H
