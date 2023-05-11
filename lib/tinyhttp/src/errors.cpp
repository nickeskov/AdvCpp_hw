#include "tinyhttp/errors.h"

#include "tinyhttp/constants.h"

namespace tinyhttp::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

HttpStandardError::HttpStandardError(constants::http_response_status status_code)
        : code_(status_code) {}

std::string_view HttpStandardError::get_text() const {
    return constants::get_http_response_status_text(code_);
}

constants::http_response_status HttpStandardError::get_code() const {
    return code_;
}

HttpNotImplemented::HttpNotImplemented()
        : HttpStandardError(constants::http_response_status::NotImplemented) {}

IoError::IoError(std::string_view what_arg) : RuntimeError(what_arg) {}

TcpError::TcpError(std::string_view what_arg) : IoError(what_arg) {}

ClosedEndpointError::ClosedEndpointError(std::string_view what_arg)
        : IoError(what_arg) {}

WriteError::WriteError(std::string_view what_arg) : IoError(what_arg) {}

ReadError::ReadError(std::string_view what_arg) : IoError(what_arg) {}

EofError::EofError(std::string_view what_arg) : ReadError(what_arg) {}

ConnectionError::ConnectionError(std::string_view what_arg)
        : RuntimeError(what_arg) {}

IoServiceError::IoServiceError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

TimeoutSetError::TimeoutSetError(std::string_view what_arg)
        : IoServiceError(what_arg) {}

ConnOpenError::ConnOpenError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

ConnCloseError::ConnCloseError(std::string_view what_arg)
        : ConnectionError(what_arg) {}

InvalidAddressError::InvalidAddressError(std::string_view what_arg)
        : ConnOpenError(what_arg) {}

ServerError::ServerError(std::string_view what_arg) : RuntimeError(what_arg) {}

EpollError::EpollError(std::string_view what_arg) : ServerError(what_arg) {}

EpollCreateError::EpollCreateError(std::string_view what_arg) : EpollError(what_arg) {}

EpollWaitError::EpollWaitError(std::string_view what_arg) : EpollError(what_arg) {}

EpollCtlError::EpollCtlError(std::string_view what_arg) : EpollError(what_arg) {}

EpollAddError::EpollAddError(std::string_view what_arg) : EpollCtlError(what_arg) {}

EpollModError::EpollModError(std::string_view what_arg) : EpollCtlError(what_arg) {}

EpollDelError::EpollDelError(std::string_view what_arg) : EpollCtlError(what_arg) {}

AcceptError::AcceptError(std::string_view what_arg) : ServerError(what_arg) {}

BindError::BindError(std::string_view what_arg) : ServerError(what_arg) {}

ListenError::ListenError(std::string_view what_arg) : ServerError(what_arg) {}

ServerCloseError::ServerCloseError(std::string_view what_arg) : ServerError(what_arg) {}

}
