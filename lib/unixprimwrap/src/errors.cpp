#include "unixprimwrap/errors.h"

#include <string>

namespace unixprimwrap::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

PipeError::PipeError(std::string_view what_arg) : RuntimeError(what_arg) {}

DescriptorError::DescriptorError(std::string_view what_arg)
        : RuntimeError(what_arg) {}


PipeCreationError::PipeCreationError(std::string_view what_arg)
        : PipeError(what_arg) {}

DupError::DupError(std::string_view what_arg) : DescriptorError(what_arg) {}

}
