#ifndef LINUXPROC_LINUXPROC_ERRORS_H
#define LINUXPROC_LINUXPROC_ERRORS_H

#include <cerrno>
#include <stdexcept>
#include <string_view>

namespace linuxproc::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class DescriptorError : public RuntimeError {
  public:
    explicit DescriptorError(std::string_view what_arg);
};

class IoError : public RuntimeError {
  public:
    explicit IoError(std::string_view what_arg);
};

class ForkError : public RuntimeError {
  public:
    explicit ForkError(std::string_view what_arg);
};

class WriteError : public IoError {
  public:
    explicit WriteError(std::string_view what_arg);
};

class WriteToClosedEndpointError : public WriteError {
  public:
    explicit WriteToClosedEndpointError(std::string_view what_arg);
};

class ReadError : public IoError {
  public:
    explicit ReadError(std::string_view what_arg);
};

class EofError : public ReadError {
  public:
    explicit EofError(std::string_view what_arg);
};

class ExecError : public RuntimeError {
  public:
    explicit ExecError(std::string_view what_arg);
};

class DupError : public DescriptorError {
  public:
    explicit DupError(std::string_view what_arg);
};

class CloseError : public RuntimeError {
  public:
    explicit CloseError(std::string_view what_arg);
};

}

#endif //LINUXPROC_LINUXPROC_ERRORS_H
