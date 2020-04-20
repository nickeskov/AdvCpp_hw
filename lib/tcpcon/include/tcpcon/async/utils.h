#ifndef TCPCON_TCPCON_ASYNC_UTILS_H
#define TCPCON_TCPCON_ASYNC_UTILS_H

#include <cinttypes>

namespace tcpcon::async::utils {

int set_nonblock(int fd, bool opt);

}

#endif //TCPCON_TCPCON_ASYNC_UTILS_H
