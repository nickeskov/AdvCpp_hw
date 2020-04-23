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


class SemaphoreError : public RuntimeError {
  public:
    explicit SemaphoreError(std::string_view what_arg);
};

class MutexError : public RuntimeError {
  public:
    explicit MutexError(std::string_view what_arg);
};

class SemaphoreInitError : public SemaphoreError {
  public:
    explicit SemaphoreInitError(std::string_view what_arg);
};

class SemaphoreWaitError : public SemaphoreError {
  public:
    explicit SemaphoreWaitError(std::string_view what_arg);
};

class SemaphoreTryWaitError : public SemaphoreError {
  public:
    explicit SemaphoreTryWaitError(std::string_view what_arg);
};

class SemaphorePostError : public SemaphoreError {
  public:
    explicit SemaphorePostError(std::string_view what_arg);
};

class MutexInitError : public MutexError {
  public:
    explicit MutexInitError(std::string_view what_arg);
};

class MutexLockError : public MutexError {
  public:
    explicit MutexLockError(std::string_view what_arg);
};

class MutexTryLockError : public MutexError {
  public:
    explicit MutexTryLockError(std::string_view what_arg);
};

class MutexUnlockError : public MutexError {
  public:
    explicit MutexUnlockError(std::string_view what_arg);
};

}

#endif //SHMEM_SHMEM_ERRORS_H
