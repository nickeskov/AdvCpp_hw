#include "trivilog/stderr_buff_logger.h"

#include <iostream>

namespace trivilog {

std::ostream &StderrBuffLogger::get_ostream() {
    return std::clog;
}

}
