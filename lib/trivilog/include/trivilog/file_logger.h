#ifndef TRIVILOG_TRIVILOG_FILE_LOGGER_H
#define TRIVILOG_TRIVILOG_FILE_LOGGER_H

#include <fstream>
#include <string>
#include <ios>

#include "base_logger.h"

namespace trivilog {

class FileLogger : public BaseLogger {
  public:
    explicit FileLogger(std::ofstream &&ofstream);

    explicit FileLogger(const std::string &filename, std::ios::openmode openmode = std::ios::out);

    FileLogger(const FileLogger &) = delete;

    FileLogger &operator=(const FileLogger &) = delete;

    ~FileLogger() noexcept override = default;

  private:
    std::ofstream ofstream_;

    [[nodiscard]] std::ostream &get_ostream() override;
};

}
#endif //TRIVILOG_TRIVILOG_FILE_LOGGER_H
