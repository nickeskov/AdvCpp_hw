#include "tinyhttp/http_query_parameters.h"
#include "tinyhttp/constants.h"
#include "tinyhttp/errors.h"

#include <algorithm>

namespace tinyhttp {

HttpQueryParameters::HttpQueryParameters(std::string_view query_string) {
    if (query_string.empty()) {
        return;
    }

    const auto params_count = 1 + std::count(
            query_string.cbegin(),
            query_string.cend(),
            constants::strings::ampersand.front()
    );

    reserve(params_count);

    size_t next = 0;

    while (next != std::string_view::npos) {
        next = query_string.find(constants::strings::ampersand);

        const auto param = query_string.substr(0, next);

        const auto sep_pos = param.find(constants::strings::equal);
        if (sep_pos == std::string_view::npos) {
            throw errors::HttpInvalidQueryString();
        }

        const auto key = query_string.substr(0, sep_pos);

        std::string_view value;

        if (sep_pos + constants::strings::equal.size() < param.size()) {
            value = param.substr(sep_pos + constants::strings::equal.size());
        }

        parameters_.emplace(key, value);

        if (next != std::string_view::npos) {
            query_string.remove_prefix(next + constants::strings::ampersand.size());
        }
    }
}

const std::string &HttpQueryParameters::at(std::string_view key) const {
    return parameters_.at(std::string(key));
}

void HttpQueryParameters::insert_or_assign(std::string_view key, std::string_view value) {
    parameters_.insert_or_assign(std::string(key), value);
}

void HttpQueryParameters::emplace(std::string_view key, std::string_view value) {
    parameters_.emplace(std::string(key), value);
}

size_t HttpQueryParameters::erase(std::string_view key) {
    return parameters_.erase(std::string(key));
}

bool HttpQueryParameters::contains(std::string_view key) const {
    return parameters_.count(std::string(key)) != 0;
}

void HttpQueryParameters::clear() noexcept {
    parameters_.clear();
}

size_t HttpQueryParameters::size() const noexcept {
    return parameters_.size();
}

bool HttpQueryParameters::empty() const noexcept {
    return parameters_.empty();
}

void HttpQueryParameters::reserve(size_t count) {
    parameters_.reserve(count);
}

HttpQueryParameters::param_storage_t::iterator HttpQueryParameters::begin() noexcept {
    return parameters_.begin();
}

HttpQueryParameters::param_storage_t::iterator HttpQueryParameters::end() noexcept {
    return parameters_.end();
}

HttpQueryParameters::param_storage_t::const_iterator HttpQueryParameters::begin() const noexcept {
    return parameters_.begin();
}

HttpQueryParameters::param_storage_t::const_iterator HttpQueryParameters::end() const noexcept {
    return parameters_.end();
}

HttpQueryParameters::param_storage_t::const_iterator HttpQueryParameters::cbegin() const noexcept {
    return parameters_.cbegin();
}

HttpQueryParameters::param_storage_t::const_iterator HttpQueryParameters::cend() const noexcept {
    return parameters_.cend();
}

std::string HttpQueryParameters::to_string() const {
    std::string buf;

    for (const auto &[key, value] : parameters_) {
        buf += key;
        buf += constants::strings::equal;
        buf += value;
        buf += constants::strings::ampersand;
    }

    if (!buf.empty()) {
        // trim last ampersand
        buf.resize(buf.size() - constants::strings::ampersand.size());
    }

    return buf;
}

}
