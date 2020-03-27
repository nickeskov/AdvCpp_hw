#include "stdout_logger.h"

#include <iostream>

namespace trivilog {

std::ostream &StdoutLogger::get_ostream() const noexcept {
    return std::cout;
}

}
