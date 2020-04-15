#include "tcpcon/errors.h"

#include <string>

namespace tcpcon::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

ServerError::ServerError(std::string_view what_arg) : RuntimeError(what_arg) {}

BindError::BindError(std::string_view what_arg) : ServerError(what_arg) {}

ListenError::ListenError(std::string_view what_arg) : ServerError(what_arg) {}

AcceptError::AcceptError(std::string_view what_arg) : ServerError(what_arg) {}

ConnectionError::ConnectionError(std::string_view what_arg)
        : RuntimeError(what_arg) {}

SocketError::SocketError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

ConnOpenError::ConnOpenError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

ConnCloseError::ConnCloseError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

InvalidAddressError::InvalidAddressError(std::string_view what_arg)
        : ConnOpenError(what_arg) {}

IoError::IoError(std::string_view what_arg) : RuntimeError(what_arg) {}


ClosedEndpointError::ClosedEndpointError(std::string_view what_arg)
        : IoError(what_arg) {}

WriteError::WriteError(std::string_view what_arg) : IoError(what_arg) {}

ReadError::ReadError(std::string_view what_arg) : IoError(what_arg) {}

EofError::EofError(std::string_view what_arg) : ReadError(what_arg) {}

}
