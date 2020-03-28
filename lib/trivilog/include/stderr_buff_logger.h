#ifndef HW_TRIVILOG_STDERR_BUFF_LOGGER_H
#define HW_TRIVILOG_STDERR_BUFF_LOGGER_H

#include "base_logger.h"

namespace trivilog {

class StderrBuffLogger : public BaseLogger {
  public:
    ~StderrBuffLogger() noexcept override = default;

  private:
    [[nodiscard]] std::ostream &get_ostream() override;
};

}

#endif //HW_TRIVILOG_STDERR_BUFF_LOGGER_H
