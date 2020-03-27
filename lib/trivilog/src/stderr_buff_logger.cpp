#include "stderr_buff_logger.h"

#include <iostream>

namespace trivilog {

std::ostream &StderrBuffLogger::get_ostream() const noexcept {
    return std::clog;
}

}
