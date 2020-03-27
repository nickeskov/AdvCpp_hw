#include "base_logger.h"

#include <chrono>
#include <iomanip>


namespace trivilog {

namespace {
decltype(auto) now_time() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    return std::put_time(std::localtime(&time), "%Y-%m-%d %T");
}

constexpr inline std::string_view trace_level_name = "[TRACE]";
constexpr inline std::string_view debug_level_name = "[DEBUG]";
constexpr inline std::string_view info_level_name =  "[INFO ]";
constexpr inline std::string_view warn_level_name =  "[WARN ]";
constexpr inline std::string_view error_level_name = "[ERROR]";
constexpr inline std::string_view fatal_level_name = "[FATAL]";
}

void BaseLogger::trace(std::string_view msg) {
    log(msg, log_level::TRACE);
}

void BaseLogger::debug(std::string_view msg) {
    log(msg, log_level::DEBUG);
}

void BaseLogger::info(std::string_view msg) {
    log(msg, log_level::INFO);
}

void BaseLogger::warn(std::string_view msg) {
    log(msg, log_level::WARN);
}

void BaseLogger::error(std::string_view msg) {
    log(msg, log_level::ERROR);
}

void BaseLogger::fatal(std::string_view msg) {
    log(msg, log_level::FATAL);
}

bool BaseLogger::set_level(log_level level) noexcept {
    if (level >= log_level::FATAL && level <= log_level::TRACE) {
        level_.store(static_cast<loglevel_t>(level));
        return true;
    }
    return false;
}

log_level BaseLogger::get_level() const noexcept {
    return static_cast<log_level>(level_.load());
}

void BaseLogger::flush() {
    get_ostream().flush();
}

void BaseLogger::log(std::string_view msg, log_level level) {
    if (get_level() >= level) {
        switch (level) {
            case log_level::TRACE: {
                log_to_ostream(trace_level_name, msg);
                break;
            }
            case log_level::DEBUG: {
                log_to_ostream(debug_level_name, msg);
                break;
            }
            case log_level::INFO: {
                log_to_ostream(info_level_name, msg);
                break;
            }
            case log_level::WARN: {
                log_to_ostream(warn_level_name, msg);
                break;
            }
            case log_level::ERROR: {
                log_to_ostream(error_level_name, msg);
                break;
            }
            case log_level::FATAL: {
                log_to_ostream(fatal_level_name, msg);
                break;
            }
        }
    }
}

void BaseLogger::log_to_ostream(std::string_view log_level_name, std::string_view msg) {
    get_ostream() << "[" << now_time() << "] " << log_level_name << " " << msg << std::endl;
}

}
