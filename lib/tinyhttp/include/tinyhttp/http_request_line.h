#ifndef TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H
#define TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H

#include <string>
#include <string_view>

#include "tinyhttp/http_query_parameters.h"

namespace tinyhttp {

class HttpRequestLine {
  public:
    explicit HttpRequestLine(std::string_view request_line);

    const std::string &get_version() const;

    const std::string &get_method() const;

    const std::string &get_url() const;

    const HttpQueryParameters &get_query_string() const;

    std::string to_string() const;

  private:
    std::string version_;
    std::string method_;
    std::string url_;

    HttpQueryParameters query_params_;
};

}

#endif //TINYHTTP_TINYHTTP_HTTP_REQUEST_LINE_H
