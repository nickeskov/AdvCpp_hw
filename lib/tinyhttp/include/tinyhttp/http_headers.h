#ifndef TINYHTTP_TINYHTTP_HTTP_HEADERS_H
#define TINYHTTP_TINYHTTP_HTTP_HEADERS_H

#include <string>
#include <string_view>
#include <unordered_map>

#include "tinyhttp/constants.h"

namespace tinyhttp {

class HttpHeaders {
  public:

    using headers_storage_t = std::unordered_map<std::string, std::string>;

    HttpHeaders() = default;

    explicit HttpHeaders(std::string_view headers_values_view);

    const std::string &at(std::string_view header) const;

    void insert_or_assign(std::string_view header, std::string_view value);

    void emplace(std::string_view header, std::string_view value);

    size_t erase(std::string_view header);

    bool contains(std::string_view header) const;

    void clear() noexcept;

    size_t size() const noexcept;

    bool empty() const noexcept;

    void reserve(size_t count);

    headers_storage_t::iterator begin() noexcept;

    headers_storage_t::iterator end() noexcept;

    headers_storage_t::const_iterator begin() const noexcept;

    headers_storage_t::const_iterator end() const noexcept;

    headers_storage_t::const_iterator cbegin() const noexcept;

    headers_storage_t::const_iterator cend() const noexcept;

    std::string to_string() const;

  private:
    headers_storage_t headers_;
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_HEADERS_H
