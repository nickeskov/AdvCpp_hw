#ifndef HW_TRIVILOG_FILE_LOGGER_H
#define HW_TRIVILOG_FILE_LOGGER_H

#include <fstream>
#include <string>

#include "base_logger.h"

namespace trivilog {

class FileLogger : public BaseLogger {
  public:
    explicit FileLogger(std::ofstream &&ofstream);

    explicit FileLogger(const std::string &filename, std::ios_base::openmode openmode = std::ios_base::out);

    FileLogger(const FileLogger &) = delete;

    FileLogger &operator=(const FileLogger &) = delete;

  private:
    std::ofstream ofstream_;

    [[nodiscard]] std::ostream &get_ostream() override;
};

}
#endif //HW_TRIVILOG_FILE_LOGGER_H
