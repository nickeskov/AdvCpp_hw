#include "errors.h"

namespace linuxproc::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(what_arg.data()) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

PipeError::PipeError(std::string_view what_arg) : RuntimeError(what_arg) {}

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

PipeCreationError::PipeCreationError(std::string_view what_arg)
        : PipeError(what_arg) {}

DupError::DupError(std::string_view what_arg) : DescriptorError(what_arg) {}

CloseError::CloseError(std::string_view what_arg) : DescriptorError(what_arg) {}

}
