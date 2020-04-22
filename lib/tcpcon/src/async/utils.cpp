#include "tcpcon/async/utils.h"

extern "C" {
#include <fcntl.h>
#include <sys/ioctl.h>
}

namespace tcpcon::async::utils {

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

}
