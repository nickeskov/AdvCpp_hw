#ifndef HW_STDOUT_LOGGER_H
#define HW_STDOUT_LOGGER_H

#include "base_logger.h"

namespace trivilog {

class StdoutLogger : public BaseLogger {
  private:
    [[nodiscard]] std::ostream &get_ostream() const noexcept override;
};

}

#endif //HW_STDOUT_LOGGER_H
