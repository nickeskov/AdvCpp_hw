#include "tcpcon/async/utils.h"

extern "C" {
#include <fcntl.h>
#include <sys/ioctl.h>
}

namespace tcpcon::async::utils {

int set_nonblock(int fd, bool opt) {
    int flags;
#ifdef O_NONBLOCK
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
        return -1;
    }
    unsigned int nonblock = (opt) ? O_NONBLOCK : ~((unsigned int) O_NONBLOCK);
    return fcntl(fd, F_SETFL, (unsigned int) flags | nonblock);
#else
    flags = (opt) ? 1 : 0;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

}
