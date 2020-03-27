#include "stdout_logger.h"

#include <iostream>

namespace trivilog {

std::ostream &StdoutLogger::get_ostream() {
    return std::cout;
}

}
