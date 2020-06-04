#include "tinyhttp/http_request.h"
#include "tinyhttp/constants.h"

#include <utility>

namespace tinyhttp {

using namespace tinyhttp::constants;

// cppcheck-suppress passedByValue; passing by value and move
HttpRequest::HttpRequest(HttpRequestLine request_line, HttpHeaders headers)
        : request_line_(std::move(request_line)), headers_(std::move(headers)) {
    if (headers_.contains(headers::content_length)) {
        content_length_ = std::stoul(headers_.at(headers::content_length));
    }
}

HttpRequestLine &HttpRequest::get_request_line() noexcept {
    return request_line_;
}

const HttpRequestLine &HttpRequest::get_request_line() const noexcept {
    return request_line_;
}

HttpHeaders &HttpRequest::get_headers() noexcept {
    return headers_;
}

const HttpHeaders &HttpRequest::get_headers() const noexcept {
    return headers_;
}

std::string &HttpRequest::get_body() noexcept {
    return body_;
}


const std::string &HttpRequest::get_body() const noexcept {
    return body_;
}

size_t HttpRequest::get_content_length() const noexcept {
    return content_length_;
}

std::string HttpRequest::to_string() const {
    std::string buf;

    buf += request_line_.to_string();
    buf += headers_.to_string();

    buf += strings::newline;

    buf += body_;

    return buf;
}

}
