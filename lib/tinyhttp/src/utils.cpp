#include "tinyhttp/utils.h"
#include "tinyhttp/constants.h"

#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

extern "C" {
#include <fcntl.h>
#include <sys/ioctl.h>
}

namespace {

constexpr char const *en_us_locale_name = "en_US.UTF8";

}

namespace tinyhttp::utils {

std::string to_lower(std::string_view view) {
    std::string lowercase_string;
    lowercase_string.reserve(view.size());

    for (const unsigned char c : view) {
        lowercase_string += static_cast<char>(std::tolower(c));
    }

    return lowercase_string;
}

std::string decode_url(std::string_view url_view) {
    std::string decoded_url;

    while (!url_view.empty()) {
        switch (url_view.front()) {
            case constants::strings::percent.front(): {
                if (url_view.size() < 3) {
                    return std::string();
                }

                const char hex_code[] = {url_view[1], url_view[2], '\0'};

                decoded_url += static_cast<char>(std::strtoul(hex_code, nullptr, 16));

                url_view.remove_prefix(3);
                break;
            }
            case constants::strings::plus.front(): {
                decoded_url += constants::strings::space;
                url_view.remove_prefix(1);
                break;
            }
            default: {
                decoded_url += url_view.front();
                url_view.remove_prefix(1);
                break;
            }
        }
    }

    return decoded_url;
}

int set_nonblock(int fd, bool opt) {
    int flags;

#ifdef O_NONBLOCK
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }

    int nonblock;
    if (opt) {
        nonblock = O_NONBLOCK;
    } else {
        // NOLINTNEXTLINE nonblock value: this is a system constant and it's can't be negative
        nonblock = ~O_NONBLOCK;
    }

    // NOLINTNEXTLINE flags: this is a system constants and it's can't be negative
    return fcntl(fd, F_SETFL, flags | nonblock);
#else
    if (opt) {
        flags = 1;
    } else {
        flags = 0;
    }
    return ioctl(fd, FIOBIO, &flags);
#endif
}

// TODO(nickeskov): use strftime function for reduce temporary strings creation and memory allocations
// TODO(nickeskov): remove stringstream and use something else, because stringstream is heavy
std::string now_time_to_str_gmt(const char *fmt, const char *locale) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss.imbue(std::locale(locale));

    tm out_date_time{};

    // NOTE(nickeskov): std::gmtime NOT THEAD SAFE, using POSIX gmtime_r to prevent data race
    gmtime_r(&time, &out_date_time);

    ss << std::put_time(&out_date_time, fmt);

    return ss.str();
}

std::string get_date_http_str() {
    // RFC 7231, 7.1.1.2: Date
    return now_time_to_str_gmt("%a, %d %b %Y %T %Z", en_us_locale_name);
}

}
