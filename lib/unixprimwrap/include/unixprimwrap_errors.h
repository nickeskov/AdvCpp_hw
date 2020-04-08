#ifndef HW_UNIXPRIMWRAP_UNIXPRIMWRAP_ERRORS_H
#define HW_UNIXPRIMWRAP_UNIXPRIMWRAP_ERRORS_H

#include <cerrno>
#include <stdexcept>
#include <string_view>

namespace unixprimwrap::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class PipeError : public RuntimeError {
  public:
    explicit PipeError(std::string_view what_arg);
};

class DescriptorError : public RuntimeError {
  public:
    explicit DescriptorError(std::string_view what_arg);
};

class PipeCreationError : public PipeError {
  public:
    explicit PipeCreationError(std::string_view what_arg);
};

class DupError : public DescriptorError {
  public:
    explicit DupError(std::string_view what_arg);
};

}

#endif //HW_LINUXPROC_LINUXPROC_ERRORS_H
