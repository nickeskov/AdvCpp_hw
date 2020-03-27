#ifndef HW_TRIVILOG_BASE_LOGGER_H
#define HW_TRIVILOG_BASE_LOGGER_H

#include <atomic>
#include <type_traits>
#include <string_view>
#include <memory>

#include "log_level.h"

namespace trivilog {

class BaseLogger {
  public:

    void fatal(std::string_view msg);

    void trace(std::string_view msg);

    void debug(std::string_view msg);

    void info(std::string_view msg);

    void warn(std::string_view msg);

    void error(std::string_view msg);

    bool set_level(log_level level) noexcept;

    [[nodiscard]] log_level get_level() const noexcept;

    virtual void flush();

  private:
    using log_level_t = std::underlying_type<log_level>::type;

    std::atomic<log_level_t> level_ = static_cast<log_level_t>(log_level::INFO);

    [[nodiscard]] virtual std::ostream &get_ostream() = 0;

    virtual void log(std::string_view msg, log_level level);

    void log_to_ostream(std::string_view log_level_name, std::string_view msg);
};

}

#endif //HW_TRIVILOG_BASE_LOGGER_H
