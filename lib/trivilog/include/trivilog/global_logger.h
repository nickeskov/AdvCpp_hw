#ifndef TRIVILOG_TRIVILOG_GLOBAL_LOGGER_H
#define TRIVILOG_TRIVILOG_GLOBAL_LOGGER_H

#include <memory>
#include <mutex>
#include <type_traits>

#include "trivilog/base_logger.h"
#include "trivilog/stdout_logger.h"

namespace trivilog::global {

class Logger final {
  public:
    [[nodiscard]] static Logger &get_instance();

    static void trace(std::string_view msg);

    static void debug(std::string_view msg);

    static void info(std::string_view msg);

    static void warn(std::string_view msg);

    static void error(std::string_view msg);

    static void fatal(std::string_view msg);

    [[nodiscard]] BaseLogger &get_global_logger();

    static void set_global_logger(std::unique_ptr<BaseLogger> new_logger);

    template<typename T, typename ...Args,
            typename = std::enable_if_t<std::is_base_of_v<BaseLogger, T>>>
    static void set_global_logger(Args &&... args) {
        get_instance().set_global_logger_impl<T>(std::forward<Args>(args)...);
    }

    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    Logger(Logger &&) = delete;

    Logger &operator=(Logger &&) = delete;

  private:
    std::unique_ptr<BaseLogger> global_logger_ptr = std::make_unique<StdoutLogger>();
    mutable std::mutex mutex_;

    Logger() = default;

    void set_global_logger_impl(std::unique_ptr<BaseLogger> new_logger);

    template<typename T, typename ...Args,
            typename = std::enable_if_t<std::is_base_of_v<BaseLogger, T>>>
    void set_global_logger_impl(Args &&... args) {
        set_global_logger_impl(std::make_unique<T>(std::forward<Args>(args)...));
    }

    void trace_impl(std::string_view msg);

    void debug_impl(std::string_view msg);

    void info_impl(std::string_view msg);

    void warn_impl(std::string_view msg);

    void error_impl(std::string_view msg);

    void fatal_impl(std::string_view msg);
};

}

#endif //TRIVILOG_TRIVILOG_GLOBAL_LOGGER_H
