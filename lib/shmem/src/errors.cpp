#include "shmem/errors.h"

#include <string>

namespace shmem::errors {

RuntimeError::RuntimeError(std::string_view what_arg)
        : std::runtime_error(std::string(what_arg)) {}

int RuntimeError::errno_code() const noexcept {
    return errno_code_;
}

LogicError::LogicError(std::string_view what_arg) :
        std::logic_error(std::string(what_arg)) {}

AllocatorError::AllocatorError(std::string_view what_arg)
        : LogicError(what_arg) {}

DeallocateError::DeallocateError(std::string_view what_arg)
        : LogicError(what_arg) {}

SemaphoreError::SemaphoreError(std::string_view what_arg)
        : RuntimeError(what_arg) {}

MutexError::MutexError(std::string_view what_arg)
        : RuntimeError(what_arg) {}

SemaphoreInitError::SemaphoreInitError(std::string_view what_arg)
        : SemaphoreError(what_arg) {}

SemaphoreWaitError::SemaphoreWaitError(std::string_view what_arg)
        : SemaphoreError(what_arg) {}

SemaphoreTryWaitError::SemaphoreTryWaitError(std::string_view what_arg)
        : SemaphoreError(what_arg) {}

SemaphorePostError::SemaphorePostError(std::string_view what_arg)
        : SemaphoreError(what_arg) {}

MutexInitError::MutexInitError(std::string_view what_arg)
        : MutexError(what_arg) {}

MutexLockError::MutexLockError(std::string_view what_arg)
        : MutexError(what_arg) {}

MutexTryLockError::MutexTryLockError(std::string_view what_arg)
        : MutexError(what_arg) {}

MutexUnlockError::MutexUnlockError(std::string_view what_arg)
        : MutexError(what_arg) {}

}
