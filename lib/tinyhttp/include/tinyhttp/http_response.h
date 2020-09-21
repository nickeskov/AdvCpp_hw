#ifndef TINYHTTP_TINYHTTP_HTTP_RESPONSE_H
#define TINYHTTP_TINYHTTP_HTTP_RESPONSE_H

#include <functional>

#include "tinyhttp/constants.h"
#include "tinyhttp/connection.h"
#include "tinyhttp/http_response_line.h"
#include "tinyhttp/http_headers.h"

namespace tinyhttp {

class HttpResponse {
  public:
    using response_sender_t = typename std::function<void(Connection &, HttpResponse &)>;

    HttpResponse();

    explicit HttpResponse(constants::http_response_status response_status,
                          constants::http_version http_version = constants::http_version::V1_1);

    HttpResponseLine &get_response_line() noexcept;

    const HttpResponseLine &get_response_line() const noexcept;

    void set_response_line(const HttpResponseLine &response_line);

    HttpHeaders &get_headers() noexcept;

    const HttpHeaders &get_headers() const noexcept;

    void reset_headers();

    void set_headers(const HttpHeaders &headers);

    const std::string &get_body() const noexcept;

    void set_body(std::string_view body);

    void append_to_body(std::string_view part);

    const response_sender_t &get_sender() const noexcept;

    void set_sender(const response_sender_t &sender);

    void set_sender(response_sender_t &&sender);

    std::string to_string() const;

  private:
    HttpResponseLine response_line_;
    HttpHeaders headers_;
    std::string body_;

    response_sender_t sender_;

    void set_basic_headers();
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_RESPONSE_H
