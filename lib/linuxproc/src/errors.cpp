#include "errors.h"
#include <cstring>
#include <cerrno>

namespace linuxproc::errors {
const char *BaseError::what() const noexcept {
    return strerror(errno);
}

}
