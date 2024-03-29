#include "trivilog/base_logger.h"

#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace trivilog {

namespace {

constexpr char const *en_us_locale_name = "en_US.UTF8";

// TODO(nickeskov): use strftime function for reduce temporary strings creation and memory allocations
// TODO(nickeskov): remove stringstream and use something else, because stringstream is heavy
std::string now_time_to_str_gmt(const char *fmt, const char *locale) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss.imbue(std::locale(locale));

    tm out_date_time{};

    // NOTE(nickeskov): std::gmtime NOT THEAD SAFE, using POSIX gmtime_r to prevent data race
    gmtime_r(&time, &out_date_time);

    ss << std::put_time(&out_date_time, fmt);

    return ss.str();
}

std::string now_time() {
    return now_time_to_str_gmt("%Y-%m-%d %T", en_us_locale_name);
}

constexpr inline std::string_view trace_level_name = "[TRACE]";
constexpr inline std::string_view debug_level_name = "[DEBUG]";
constexpr inline std::string_view info_level_name = "[INFO ]";
constexpr inline std::string_view warn_level_name = "[WARN ]";
constexpr inline std::string_view error_level_name = "[ERROR]";
constexpr inline std::string_view crit_level_name = "[CRIT ]";
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

void BaseLogger::crit(std::string_view msg) {
    log(msg, log_level::CRIT);
}

void BaseLogger::fatal(std::string_view msg) {
    log(msg, log_level::FATAL);
}

void BaseLogger::set_level(log_level level) noexcept {
    level_.store(level);
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
            case log_level::CRIT: {
                log_to_ostream(crit_level_name, msg);
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
