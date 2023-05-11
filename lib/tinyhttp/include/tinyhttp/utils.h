#ifndef TINYHTTP_TINYHTTP_UTILS_H
#define TINYHTTP_TINYHTTP_UTILS_H

#include <string>
#include <string_view>

namespace tinyhttp::utils {

std::string to_lower(std::string_view view);

std::string decode_url(std::string_view url_view);

int set_nonblock(int fd, bool opt);

std::string now_time_to_str_gmt(const char *fmt, const char *locale);

std::string get_date_http_str();

}

#endif //TINYHTTP_TINYHTTP_UTILS_H
