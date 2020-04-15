#include "linuxproc/errors.h"

#include <string>

namespace linuxproc::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

DescriptorError::DescriptorError(std::string_view what_arg)
        : RuntimeError(what_arg) {}

IoError::IoError(std::string_view what_arg) : RuntimeError(what_arg) {}

ForkError::ForkError(std::string_view what_arg) : RuntimeError(what_arg) {}

WriteError::WriteError(std::string_view what_arg) : IoError(what_arg) {}

WriteToClosedEndpointError::WriteToClosedEndpointError(std::string_view what_arg)
        : WriteError(what_arg) {}

ReadError::ReadError(std::string_view what_arg) : IoError(what_arg) {}

EofError::EofError(std::string_view what_arg) : ReadError(what_arg) {}

ExecError::ExecError(std::string_view what_arg) : RuntimeError(what_arg) {}

DupError::DupError(std::string_view what_arg) : DescriptorError(what_arg) {}

CloseError::CloseError(std::string_view what_arg) : RuntimeError(what_arg) {}

}
