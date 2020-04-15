#include "trivilog/file_logger.h"

#include <utility>

namespace trivilog {

FileLogger::FileLogger(std::ofstream &&ofstream) : ofstream_(std::move(ofstream)) {}

FileLogger::FileLogger(const std::string &filename, std::ios_base::openmode openmode)
        : ofstream_(filename, openmode) {}

std::ostream &FileLogger::get_ostream() {
    return ofstream_;
}

}
