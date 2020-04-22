#include "shmem/errors.h"

#include <string>

namespace shmem::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

LogicError::LogicError(std::string_view what_arg) :
        std::logic_error(std::string(what_arg)) {}

AllocatorError::AllocatorError(std::string_view what_arg)
        : LogicError(what_arg) {}

DeallocateError::DeallocateError(std::string_view what_arg)
        : LogicError(what_arg) {}

}
