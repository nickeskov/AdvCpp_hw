#ifndef TINYHTTP_TINYHTTP_CONSTANTS_H
#define TINYHTTP_TINYHTTP_CONSTANTS_H

#include <string_view>

namespace tinyhttp::constants::strings {

constexpr std::string_view newline = std::string_view("\r\n");
constexpr std::string_view headers_end = std::string_view("\r\n\r\n");
constexpr std::string_view colon = std::string_view(":");
constexpr std::string_view space = std::string_view(" ");
constexpr std::string_view question = std::string_view("?");
constexpr std::string_view ampersand = std::string_view("&");
constexpr std::string_view equal = std::string_view("=");
constexpr std::string_view percent = std::string_view("%");
constexpr std::string_view plus = std::string_view("+");
constexpr std::string_view slash = std::string_view("/");
constexpr std::string_view http_upper = std::string_view("HTTP");
constexpr std::string_view http_lower = std::string_view("http");

}

namespace tinyhttp::constants::headers {

constexpr std::string_view content_length = std::string_view("Content-Length");

}


#endif //TINYHTTP_TINYHTTP_CONSTANTS_H
