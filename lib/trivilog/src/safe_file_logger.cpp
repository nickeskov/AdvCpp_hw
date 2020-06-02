#include "trivilog/safe_file_logger.h"

namespace trivilog {

SafeFileLogger::SafeFileLogger(std::ofstream &&ofstream) : FileLogger(std::move(ofstream)) {}

SafeFileLogger::SafeFileLogger(const std::string &filename, std::ios::openmode openmode)
        : FileLogger(filename, openmode) {}

void SafeFileLogger::log_to_ostream(std::string_view log_level_name, std::string_view msg) {
    std::lock_guard<std::mutex> guard(mutex_);
    BaseLogger::log_to_ostream(log_level_name, msg);
}

}
