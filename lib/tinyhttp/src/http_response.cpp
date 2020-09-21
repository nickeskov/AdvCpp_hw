#include "tinyhttp/http_response.h"
#include "tinyhttp/utils.h"

namespace tinyhttp {

HttpResponse::HttpResponse() {
    set_basic_headers();
}

HttpResponse::HttpResponse(constants::http_response_status response_status,
                           constants::http_version http_version)
        : response_line_(response_status, http_version) {
    set_basic_headers();
}

HttpResponseLine &HttpResponse::get_response_line() noexcept {
    return response_line_;
}

const HttpResponseLine &HttpResponse::get_response_line() const noexcept {
    return response_line_;
}

void HttpResponse::set_response_line(const HttpResponseLine &response_line) {
    response_line_ = response_line;
}

HttpHeaders &HttpResponse::get_headers() noexcept {
    return headers_;
}

const HttpHeaders &HttpResponse::get_headers() const noexcept {
    return headers_;
}

void HttpResponse::set_headers(const HttpHeaders &headers) {
    headers_ = headers;
}

const std::string &HttpResponse::get_body() const noexcept {
    return body_;
}

void HttpResponse::set_body(std::string_view body) {
    body_.clear();
    append_to_body(body);
}

void HttpResponse::append_to_body(std::string_view part) {
    body_ += part;
    headers_.insert_or_assign(constants::headers::content_length,
                              std::to_string(body_.size()));
}

const HttpResponse::response_sender_t &HttpResponse::get_sender() const noexcept {
    return sender_;
}

void HttpResponse::set_sender(const HttpResponse::response_sender_t &sender) {
    sender_ = sender;
}

std::string HttpResponse::to_string() const {
    std::string buf;
    buf.reserve(body_.size());

    buf += response_line_.to_string();

    buf += constants::strings::newline;

    buf += headers_.to_string();

    buf += constants::strings::headers_end;

    buf += body_;

    return buf;
}

void HttpResponse::set_basic_headers() {
    headers_.emplace(constants::headers::server, constants::server_name);
    headers_.emplace(constants::headers::date, utils::get_date_http_str());

    // TODO(nickeskov): hardcoded connection type
    if (response_line_.get_http_version() > constants::http_version::V0_9) {
        headers_.emplace(constants::headers::connection, "closed");
    }
}

}
