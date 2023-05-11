#ifndef TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H
#define TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H

#include <string>
#include <string_view>

#include "tinyhttp/http_query_parameters.h"
#include "tinyhttp/constants.h"

namespace tinyhttp {

class HttpRequestLine {
  public:
    explicit HttpRequestLine(std::string_view request_line);

    constants::http_version get_version() const;

    constants::http_method get_method() const;

    const std::string &get_url() const;

    const HttpQueryParameters &get_query_string() const;

    std::string to_string() const;

  private:
    constants::http_version version_;
    constants::http_method method_;
    std::string url_;

    HttpQueryParameters query_params_;
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H
