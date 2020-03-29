#include "errors.h"

namespace linuxproc::errors {

RuntimeError::RuntimeError(const std::string_view &what_arg)
        : std::runtime_error(what_arg.data()) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

PipeError::PipeError(const std::string_view &what_arg) : RuntimeError(what_arg) {}

DescriptorError::DescriptorError(const std::string_view &what_arg)
        : RuntimeError(what_arg) {}

IoError::IoError(const std::string_view &what_arg) : RuntimeError(what_arg) {}

ForkError::ForkError(const std::string_view &what_arg) : RuntimeError(what_arg) {}

WriteError::WriteError(const std::string_view &what_arg) : IoError(what_arg) {}

WriteToClosedEndpointError::WriteToClosedEndpointError(const std::string_view &what_arg)
        : WriteError(what_arg) {}

ReadError::ReadError(const std::string_view &what_arg) : IoError(what_arg) {}

EofError::EofError(const std::string_view &what_arg) : ReadError(what_arg) {}

ExecError::ExecError(const std::string_view &what_arg) : RuntimeError(what_arg) {}

PipeCreationError::PipeCreationError(const std::string_view &what_arg)
        : PipeError(what_arg) {}

DupError::DupError(const std::string_view &what_arg) : DescriptorError(what_arg) {}

CloseError::CloseError(const std::string_view &what_arg) : DescriptorError(what_arg) {}

}
