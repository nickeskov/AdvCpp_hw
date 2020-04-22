#ifndef SHMEM_SHMEM_ERRORS_H
#define SHMEM_SHMEM_ERRORS_H

#include <cerrno>
#include <stdexcept>
#include <string_view>

namespace shmem::errors {

class RuntimeError : public std::runtime_error {
  public:
    explicit RuntimeError(std::string_view what_arg);

    [[nodiscard]] int errno_code() const noexcept;

    ~RuntimeError() override = default;

  private:
    int errno_code_ = errno;
};

class LogicError : public std::logic_error {
  public:
    explicit LogicError(std::string_view what_arg);
};

class AllocatorError : public LogicError {
  public:
    explicit AllocatorError(std::string_view what_arg);
};

class DeallocateError : public LogicError {
  public:
    explicit DeallocateError(std::string_view what_arg);
};

}

#endif //SHMEM_SHMEM_ERRORS_H
