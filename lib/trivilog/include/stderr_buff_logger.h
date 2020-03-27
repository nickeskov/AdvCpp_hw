#ifndef HW_STDERR_BUFF_LOGGER_H
#define HW_STDERR_BUFF_LOGGER_H

#include "base_logger.h"

namespace trivilog {

class StderrBuffLogger : public BaseLogger {
  private:
    [[nodiscard]] std::ostream &get_ostream() const noexcept override;
};

}

#endif //HW_STDERR_BUFF_LOGGER_H
