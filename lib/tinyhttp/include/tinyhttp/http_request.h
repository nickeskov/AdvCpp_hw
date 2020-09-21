#ifndef TINYHTTP_TINYHTTP_HTTP_REQUEST_H
#define TINYHTTP_TINYHTTP_HTTP_REQUEST_H

#include "tinyhttp/http_headers.h"
#include "tinyhttp/http_request_line.h"

#include <string>
#include <string_view>

namespace tinyhttp {

// TODO(nickeskov): create method for set body into request object
class HttpRequest {
  public:

    HttpRequest(HttpRequestLine request_line, HttpHeaders headers);

    HttpRequest(HttpRequestLine request_line, HttpHeaders headers, std::string_view body);

    HttpRequestLine &get_request_line() noexcept;

    const HttpRequestLine &get_request_line() const noexcept;

    HttpHeaders &get_headers() noexcept;

    const HttpHeaders &get_headers() const noexcept;

    std::string &get_body() noexcept;

    void set_body(std::string_view body);

    void set_body(std::string &&body) noexcept;

    const std::string &get_body() const noexcept;

    size_t get_content_length() const noexcept;

    std::string to_string() const;

  private:
    HttpRequestLine request_line_;
    HttpHeaders headers_;
    std::string body_;
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_REQUEST_H
