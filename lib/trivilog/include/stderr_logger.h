#ifndef HW_TRIVILOG_STDERR_LOGGER_H
#define HW_TRIVILOG_STDERR_LOGGER_H

#include "base_logger.h"

namespace trivilog {

class StderrLogger : public BaseLogger {
  public:
    ~StderrLogger() noexcept override = default;

  private:
    [[nodiscard]] std::ostream &get_ostream() override;
};

}

#endif //HW_TRIVILOG_STDERR_LOGGER_H
