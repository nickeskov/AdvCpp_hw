#include "tinyhttp/http_response_line.h"
#include "tinyhttp/errors.h"

#include <string>
#include <tinyhttp/http_request_line.h>

namespace tinyhttp {

HttpResponseLine::HttpResponseLine(constants::http_response_status response_status,
                                   constants::http_version http_version)
        : http_version_(http_version), response_status_(response_status) {}

constants::http_version HttpResponseLine::get_http_version() const noexcept {
    return http_version_;
}

constants::http_response_status HttpResponseLine::get_response_status() const noexcept {
    return response_status_;
}

void HttpResponseLine::set_http_version(constants::http_version http_version) noexcept {
    http_version_ = http_version;
}

void HttpResponseLine::set_response_status(constants::http_response_status response_status) noexcept {
    response_status_ = response_status;
}

std::string HttpResponseLine::to_string() const {
    auto version_text = constants::get_http_version_text(http_version_);
    if (version_text.empty()) {
        throw errors::HttpVersionInvalid();
    }

    std::string response_line;

    response_line += constants::strings::http_upper;
    response_line += constants::strings::slash;
    response_line += version_text;

    response_line += constants::strings::space;

    response_line += std::to_string(static_cast<uint16_t>(response_status_));

    response_line += constants::strings::space;

    response_line += constants::get_http_response_status_text(response_status_);

    return response_line;
}

}
