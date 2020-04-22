#include "concurrentsync/mutex.h"
#include "concurrentsync/errors.h"

#include <cerrno>
#include <utility>

namespace concurrentsync {

Mutex::Mutex(bool is_process_mutex): is_empty_(false) {
    if (::sem_init(&mutex_, static_cast<int>(is_process_mutex), 1) < 0) {
        throw errors::MutexInitError("cannot init unix mutex");
    }
}

Mutex::Mutex(Mutex &&other) noexcept {
    swap(other);
}

Mutex &Mutex::operator=(Mutex &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    Mutex().swap(*this);
    swap(other);
    return *this;
}

void Mutex::swap(Mutex &other) noexcept {
    std::swap(mutex_, other.mutex_);
    std::swap(is_empty_, other.is_empty_);
}

void Mutex::lock() {
    if (::sem_wait(&mutex_)) {
        throw errors::MutexLockError("mutex lock error");
    }
}

bool Mutex::try_lock() {
    if (::sem_trywait(&mutex_) < 0) {
        if (errno != EAGAIN) {
            throw errors::MutexTryLockError("mutex try_lock error");
        }
        return false;
    }
    return true;
}

void Mutex::unlock() {
    if (::sem_post(&mutex_)) {
        throw errors::MutexUnlockError("mutex unlock error");
    }
}

Mutex::~Mutex() noexcept {
    if (!is_empty_) {
        ::sem_destroy(&mutex_);
    }
}

}
