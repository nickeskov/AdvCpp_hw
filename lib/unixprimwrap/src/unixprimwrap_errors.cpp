#include "unixprimwrap_errors.h"

namespace unixprimwrap::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(what_arg.data()) {}

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
