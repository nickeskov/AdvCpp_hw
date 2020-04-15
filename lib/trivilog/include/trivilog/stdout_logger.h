#ifndef TRIVILOG_TRIVILOG_STDOUT_LOGGER_H
#define TRIVILOG_TRIVILOG_STDOUT_LOGGER_H

#include "base_logger.h"

namespace trivilog {

class StdoutLogger : public BaseLogger {
  public:
    ~StdoutLogger() noexcept override = default;
  private:
    [[nodiscard]] std::ostream &get_ostream() override;
};

}

#endif //TRIVILOG_TRIVILOG_STDOUT_LOGGER_H
