#ifndef HW_GLOBAL_LOGGER_H
#define HW_GLOBAL_LOGGER_H

#include "base_logger.h"
#include "stdout_logger.h"
#include <memory>
#include <mutex>
#include <type_traits>

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

    void set_global_logger(std::unique_ptr<BaseLogger> new_logger);

    template<typename T, typename ...Args>
    void set_global_logger(Args &&... args) {
        std::lock_guard guard(mutex_);
        global_logger_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    }

    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    Logger(Logger &&) = delete;

    Logger &operator=(Logger &&) = delete;

  private:
    std::unique_ptr<BaseLogger> global_logger_ptr = std::make_unique<StdoutLogger>();
    mutable std::mutex mutex_;

    Logger() = default;

    void trace_impl(std::string_view msg);

    void debug_impl(std::string_view msg);

    void info_impl(std::string_view msg);

    void warn_impl(std::string_view msg);

    void error_impl(std::string_view msg);

    void fatal_impl(std::string_view msg);

};

}

#endif //HW_GLOBAL_LOGGER_H
