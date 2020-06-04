#ifndef TINYHTTP_TINYHTTP_UTILS_H
#define TINYHTTP_TINYHTTP_UTILS_H

#include <string>
#include <string_view>

namespace tinyhttp::utils {

std::string to_lower(std::string_view view);

std::string decode_url(std::string_view url_view);

int set_nonblock(int fd, bool opt);

}

#endif //TINYHTTP_TINYHTTP_UTILS_H
