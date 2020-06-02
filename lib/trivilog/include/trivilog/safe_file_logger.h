#ifndef TRIVILOG_TRIVILOG_SAFE_FILE_LOGGER_H
#define TRIVILOG_TRIVILOG_SAFE_FILE_LOGGER_H

#include <mutex>

#include "trivilog/file_logger.h"

namespace trivilog {

class SafeFileLogger : public FileLogger {
  public:

    explicit SafeFileLogger(std::ofstream &&ofstream);

    explicit SafeFileLogger(const std::string &filename, std::ios::openmode openmode = std::ios::out);

    SafeFileLogger(const SafeFileLogger &) = delete;

    SafeFileLogger &operator=(const SafeFileLogger &) = delete;

    ~SafeFileLogger() noexcept override = default;

  protected:
    void log_to_ostream(std::string_view log_level_name, std::string_view msg) override;

  private:
    std::mutex mutex_;
};

}

#endif //TRIVILOG_TRIVILOG_SAFE_FILE_LOGGER_H
