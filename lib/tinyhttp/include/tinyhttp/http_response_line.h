#ifndef TINYHTTP_TINYHTTP_HTTP_RESPONSE_LINE_H
#define TINYHTTP_TINYHTTP_HTTP_RESPONSE_LINE_H

#include "tinyhttp/constants.h"

namespace tinyhttp {

class HttpResponseLine {
  public:

    HttpResponseLine() = default;

    HttpResponseLine(constants::http_response_status response_status,
                              constants::http_version http_version);

    [[nodiscard]] constants::http_version get_http_version() const noexcept;

    [[nodiscard]] constants::http_response_status get_response_status() const noexcept;

    void set_http_version(constants::http_version http_version) noexcept;

    void set_response_status(constants::http_response_status response_status) noexcept;

    [[nodiscard]] std::string to_string() const;

  private:
    constants::http_version http_version_ = constants::http_version::V1_1;
    constants::http_response_status response_status_ = constants::http_response_status::OK;

};

}

#endif //TINYHTTP_TINYHTTP_HTTP_RESPONSE_LINE_H
