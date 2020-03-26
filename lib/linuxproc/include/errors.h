#ifndef HW_ERRORS_H
#define HW_ERRORS_H

#include <exception>

namespace linuxproc::errors {

class BaseError : public std::exception {
  public:
    [[nodiscard]] const char *what() const noexcept override;
};

class PipeError : public BaseError {};

class DescriptorError : public BaseError {};

class IoError : public BaseError {};

class ForkError : public BaseError {};

class WriteError : public IoError {};

class ReadError : public IoError {};

class EofError : public IoError {};

class ExecError : public BaseError {};

class PipeCreationException : public PipeError {};

class DupError : public DescriptorError {};

class CloseError : public DescriptorError {};

}

#endif //HW_ERRORS_H
