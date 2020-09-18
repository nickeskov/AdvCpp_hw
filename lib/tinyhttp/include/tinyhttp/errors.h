#ifndef TINYHTTP_TINYHTTP_ERRORS_H
#define TINYHTTP_TINYHTTP_ERRORS_H

#include <exception>
#include <stdexcept>
#include <string_view>
#include <cerrno>

namespace tinyhttp::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class HttpBaseError : public std::exception {
};

class HttpSyntaxError : public HttpBaseError {
};

class HttpInvalidHeaders : public HttpSyntaxError {
};

class HttpInvalidQueryString : public HttpSyntaxError {
};

class HttpInvalidRequestLine : public HttpSyntaxError {
};

class HttpStandardError : public HttpBaseError {
  public:
    HttpStandardError(int code, std::string_view text);

    [[nodiscard]] const std::string &get_text() const;

    [[nodiscard]] int get_code() const;

    ~HttpStandardError() override = default;

  private:
    int code_{};
    std::string text_;
};

class HttpNotImplemented : public HttpStandardError {
  public:
    HttpNotImplemented();
    explicit HttpNotImplemented(std::string_view text);

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

class TcpError : public IoError {
  public:
    explicit TcpError(std::string_view what_arg);
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

}

#endif //TINYHTTP_TINYHTTP_ERRORS_H
