#ifndef TINYHTTP_TINYHTTP_HTTP_QUERY_PARAMETERS_H
#define TINYHTTP_TINYHTTP_HTTP_QUERY_PARAMETERS_H

#include <string>
#include <string_view>
#include <unordered_map>

namespace tinyhttp {

class HttpQueryParameters {
  public:

    using param_storage_t = std::unordered_map<std::string, std::string>;

    HttpQueryParameters() = default;

    explicit HttpQueryParameters(std::string_view query_string);

    const std::string &at(std::string_view key) const;

    void insert_or_assign(std::string_view key, std::string_view value);

    void emplace(std::string_view key, std::string_view value);

    size_t erase(std::string_view key);

    bool contains(std::string_view key) const;

    void clear() noexcept;

    size_t size() const noexcept;

    bool empty() const noexcept;

    void reserve(size_t count);

    param_storage_t::iterator begin() noexcept;

    param_storage_t::iterator end() noexcept;

    param_storage_t::const_iterator begin() const noexcept;

    param_storage_t::const_iterator end() const noexcept;

    param_storage_t::const_iterator cbegin() const noexcept;

    param_storage_t::const_iterator cend() const noexcept;

    std::string to_string() const;

  private:
    param_storage_t parameters_;
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_QUERY_PARAMETERS_H
