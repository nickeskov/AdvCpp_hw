#ifndef TRIVILOG_TRIVILOG_STDERR_LOGGER_H
#define TRIVILOG_TRIVILOG_STDERR_LOGGER_H

#include "trivilog/base_logger.h"

namespace trivilog {

class StderrLogger : public BaseLogger {
  public:
    ~StderrLogger() noexcept override = default;

  private:
    [[nodiscard]] std::ostream &get_ostream() override;
};

}

#endif //TRIVILOG_TRIVILOG_STDERR_LOGGER_H
