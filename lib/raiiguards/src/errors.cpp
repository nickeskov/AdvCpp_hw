#include "raiiguards/errors.h"

#include <string>

namespace raiiguards::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

}
