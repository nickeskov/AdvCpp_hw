#include "tinyhttp/http_headers.h"
#include "tinyhttp/utils.h"
#include "tinyhttp/errors.h"

#include <algorithm>

namespace tinyhttp {

HttpHeaders::HttpHeaders(std::string_view headers_values_view) {
    const auto headers_count = std::count(
            headers_values_view.cbegin(),
            headers_values_view.cend(),
            constants::strings::newline.back()
    );

    reserve(headers_count);

    for (size_t next = headers_values_view.find(constants::strings::newline);
         next != std::string_view::npos;
         next = headers_values_view.find(constants::strings::newline)) {

        const std::string_view header_value_view = headers_values_view.substr(0, next);
        if (header_value_view.empty()) {
            break; // if we get "\r\n"
        }

        size_t colon_pos = header_value_view.find(constants::strings::colon);
        if (colon_pos == std::string_view::npos) {
            throw errors::HttpInvalidHeaders();
        }

        const std::string_view header_view = header_value_view.substr(0, colon_pos);
        std::string_view value_view;

        if (colon_pos + constants::strings::colon.size() < header_value_view.size()) {
            value_view = header_value_view.substr(colon_pos + constants::strings::colon.size());

            const auto not_space_pos = std::min(
                    value_view.find_first_not_of(constants::strings::space),
                    value_view.size()
            );

            value_view.remove_prefix(not_space_pos);
        }

        headers_.emplace(utils::to_lower(header_view), value_view);

        headers_values_view.remove_prefix(next + constants::strings::newline.size());
    }
}

const std::string &HttpHeaders::at(std::string_view header) const {
    return headers_.at(utils::to_lower(header));
}

void HttpHeaders::insert_or_assign(std::string_view header, std::string_view value) {
    headers_.insert_or_assign(utils::to_lower(header), value);
}

void HttpHeaders::emplace(std::string_view header, std::string_view value) {
    headers_.emplace(utils::to_lower(header), value);
}

size_t HttpHeaders::erase(std::string_view header) {
    return headers_.erase(utils::to_lower(header));
}

bool HttpHeaders::contains(std::string_view header) const {
    return headers_.count(utils::to_lower(header)) != 0;
}

void HttpHeaders::clear() noexcept {
    headers_.clear();
}

size_t HttpHeaders::size() const noexcept {
    return headers_.size();
}

bool HttpHeaders::empty() const noexcept {
    return headers_.empty();
}

void HttpHeaders::reserve(size_t count) {
    headers_.reserve(count);
}

HttpHeaders::headers_storage_t::iterator HttpHeaders::begin() noexcept {
    return headers_.begin();
}

HttpHeaders::headers_storage_t::iterator HttpHeaders::end() noexcept {
    return headers_.end();
}

HttpHeaders::headers_storage_t::const_iterator HttpHeaders::begin() const noexcept {
    return headers_.begin();
}

HttpHeaders::headers_storage_t::const_iterator HttpHeaders::end() const noexcept {
    return headers_.end();
}

HttpHeaders::headers_storage_t::const_iterator HttpHeaders::cbegin() const noexcept {
    return headers_.cbegin();
}

HttpHeaders::headers_storage_t::const_iterator HttpHeaders::cend() const noexcept {
    return headers_.cend();
}

std::string HttpHeaders::to_string() const {
    std::string buf;

    for (const auto &[header, value] : headers_) {
        buf += header;
        buf += constants::strings::colon;
        buf += constants::strings::space;
        buf += value;
        buf += constants::strings::newline;
    }

    return buf;
}

}
