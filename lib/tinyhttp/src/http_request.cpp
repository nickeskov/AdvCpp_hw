#include "tinyhttp/http_request.h"
#include "tinyhttp/constants.h"

#include <utility>

namespace tinyhttp {

using namespace tinyhttp::constants;

// cppcheck-suppress passedByValue ; passing by value and move
HttpRequest::HttpRequest(HttpRequestLine request_line, HttpHeaders headers)
        : request_line_(std::move(request_line)), headers_(std::move(headers)) {}

// cppcheck-suppress passedByValue ; passing by value and move
HttpRequest::HttpRequest(HttpRequestLine request_line, HttpHeaders headers, std::string_view body)
        : request_line_(std::move(request_line)), headers_(std::move(headers)), body_(body) {}

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

void HttpRequest::set_body(std::string_view body) {
    body_ = body;
}

void HttpRequest::set_body(std::string &&body) noexcept {
    body_ = std::move(body);
}

size_t HttpRequest::get_content_length() const noexcept {
    return body_.size();
}

std::string HttpRequest::to_string() const {
    std::string buf;
    buf.reserve(body_.size());

    buf += request_line_.to_string();

    buf += strings::newline;

    buf += headers_.to_string();

    buf += strings::headers_end;

    buf += body_;

    return buf;
}

}
