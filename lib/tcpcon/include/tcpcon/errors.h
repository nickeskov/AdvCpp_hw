#ifndef TCPCON_TCPCON_ERRORS_H
#define TCPCON_TCPCON_ERRORS_H

#include <stdexcept>
#include <cerrno>
#include <string_view>

namespace tcpcon::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class ServerError : public RuntimeError {
  public:
    explicit ServerError(std::string_view what_arg);
};

class BindError : public ServerError {
  public:
    explicit BindError(std::string_view what_arg);
};

class ListenError : public ServerError {
  public:
    explicit ListenError(std::string_view what_arg);
};

class AcceptError : public ServerError {
  public:
    explicit AcceptError(std::string_view what_arg);
};

class ServerCloseError : public ServerError {
  public:
    explicit ServerCloseError(std::string_view what_arg);
};

class ConnectionError : public RuntimeError {
  public:
    explicit ConnectionError(std::string_view what_arg);
};

class IoServiceError : public ConnectionError {
  public:
    explicit IoServiceError(std::string_view what_arg);
};

class TimeoutSetError : public IoServiceError {
  public:
    explicit TimeoutSetError(std::string_view what_arg);
};

class ConnOpenError : public ConnectionError {
  public:
    explicit ConnOpenError(std::string_view what_arg);
};

class ConnCloseError : public ConnectionError {
  public:
    explicit ConnCloseError(std::string_view what_arg);
};

class InvalidAddressError : public ConnOpenError {
  public:
    explicit InvalidAddressError(std::string_view what_arg);
};

class IoError : public RuntimeError {
  public:
    explicit IoError(std::string_view what_arg);
};

class ClosedEndpointError : public IoError {
  public:
    explicit ClosedEndpointError(std::string_view what_arg);
};

class WriteError : public IoError {
  public:
    explicit WriteError(std::string_view what_arg);
};

class ReadError : public IoError {
  public:
    explicit ReadError(std::string_view what_arg);
};

class EofError : public ReadError {
  public:
    explicit EofError(std::string_view what_arg);
};

}
#endif //TCPCON_TCPCON_ERRORS_H
