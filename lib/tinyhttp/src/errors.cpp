#include "tinyhttp/errors.h"

#include <unordered_map>

namespace tinyhttp::errors {

namespace {

enum codes {
    status_internal_server_error = 500,
    status_not_implemented = 501,
};

const std::unordered_map<int, std::string> code_msg_map = {
        {status_internal_server_error, "Internal Server Error"},
        {status_not_implemented,       "Not Implemented"},
};

}


RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

HttpStandardError::HttpStandardError(int code, std::string_view text)
        : code_(code), text_(text) {}

const std::string &HttpStandardError::get_text() const {
    return text_;
}

int HttpStandardError::get_code() const {
    return code_;
}

HttpNotImplemented::HttpNotImplemented()
        : HttpStandardError(status_not_implemented, code_msg_map.at(status_not_implemented)) {}

HttpNotImplemented::HttpNotImplemented(std::string_view text)
        : HttpStandardError(status_not_implemented, text) {}

}
