#ifndef HW_ERRORS_H
#define HW_ERRORS_H

#include <cerrno>
#include <stdexcept>
#include <string_view>

namespace linuxproc::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(const std::string_view &what_arg);

    [[nodiscard]] int errno_code() const noexcept;

  private:
    int errno_code_ = errno;
};

class PipeError : public RuntimeError {
  public:
    explicit PipeError(const std::string_view &what_arg);
};

class DescriptorError : public RuntimeError {
  public:
    explicit DescriptorError(const std::string_view &what_arg);
};

class IoError : public RuntimeError {
  public:
    explicit IoError(const std::string_view &what_arg);
};

class ForkError : public RuntimeError {
  public:
    explicit ForkError(const std::string_view &what_arg);
};

class WriteError : public IoError {
  public:
    explicit WriteError(const std::string_view &what_arg);
};

class WriteToClosedEndpointError : public WriteError {
  public:
    explicit WriteToClosedEndpointError(const std::string_view &what_arg);
};

class ReadError : public IoError {
  public:
    explicit ReadError(const std::string_view &what_arg);
};

class EofError : public ReadError {
  public:
    explicit EofError(const std::string_view &what_arg);
};

class ExecError : public RuntimeError {
  public:
    explicit ExecError(const std::string_view &what_arg);
};

class PipeCreationError : public PipeError {
  public:
    explicit PipeCreationError(const std::string_view &what_arg);
};

class DupError : public DescriptorError {
  public:
    explicit DupError(const std::string_view &what_arg);
};

class CloseError : public DescriptorError {
  public:
    explicit CloseError(const std::string_view &what_arg);
};

}

#endif //HW_ERRORS_H
