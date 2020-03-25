#include "errors.h"
#include <cstring>
#include <cerrno>

namespace linuxproc {

const char *BaseError::what() const noexcept {
    return strerror(errno);
}

}