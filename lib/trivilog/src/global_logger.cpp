#include "global_logger.h"

namespace trivilog::global {

Logger &Logger::get_instance() {
    static Logger instance;
    return instance;
}

BaseLogger &Logger::get_global_logger() {
    std::lock_guard guard(mutex_);
    return *global_logger_ptr;
}

void Logger::trace(std::string_view msg) {
    get_instance().trace_impl(msg);
}

void Logger::debug(std::string_view msg) {
    get_instance().debug_impl(msg);
}

void Logger::info(std::string_view msg) {
    get_instance().info_impl(msg);
}

void Logger::warn(std::string_view msg) {
    get_instance().warn_impl(msg);
}

void Logger::error(std::string_view msg) {
    get_instance().error_impl(msg);
}

void Logger::fatal(std::string_view msg) {
    get_instance().fatal_impl(msg);
}

void Logger::set_global_logger(std::unique_ptr<BaseLogger> new_logger) {
    get_instance().set_global_logger_impl(std::move(new_logger));
}

void Logger::set_global_logger_impl(std::unique_ptr<BaseLogger> new_logger) {
    std::lock_guard guard(mutex_);
    global_logger_ptr.swap(new_logger);
}

void Logger::trace_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->trace(msg);
}

void Logger::debug_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->debug(msg);
}

void Logger::info_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->info(msg);
}

void Logger::warn_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->warn(msg);
}

void Logger::error_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->error(msg);
}

void Logger::fatal_impl(std::string_view msg) {
    std::lock_guard guard(mutex_);
    global_logger_ptr->fatal(msg);
}

}
