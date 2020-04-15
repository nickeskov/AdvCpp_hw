#include "trivilog/stderr_logger.h"

#include <iostream>

namespace trivilog {

std::ostream &StderrLogger::get_ostream() {
    return std::cerr;
}

}
