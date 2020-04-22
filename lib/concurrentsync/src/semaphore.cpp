#include "concurrentsync/semaphore.h"
#include "concurrentsync/errors.h"

#include <cerrno>
#include <utility>

namespace concurrentsync {

Semaphore::Semaphore(bool is_process_semaphore, unsigned int value): is_empty_(false) {
    if (::sem_init(&semaphore_, static_cast<int>(is_process_semaphore), value) < 0) {
        throw errors::SemaphoreInitError("cannot init unix semaphore");
    }
}

Semaphore::Semaphore(Semaphore &&other) noexcept {
    swap(other);
}

Semaphore &Semaphore::operator=(Semaphore &&other) noexcept {
    if (this == &other) {
        return *this;
    }
    Semaphore().swap(*this);
    swap(other);
    return *this;
}

void Semaphore::swap(Semaphore &other) noexcept {
    std::swap(semaphore_, other.semaphore_);
    std::swap(is_empty_, other.is_empty_);
}

void Semaphore::wait() {
    if (::sem_wait(&semaphore_)) {
        throw errors::SemaphoreWaitError("semaphore wait error");
    }
}

bool Semaphore::try_wait() {
    if (::sem_trywait(&semaphore_) < 0) {
        if (errno != EAGAIN) {
            throw errors::SemaphoreTryWaitError("semaphore try_wait error");
        }
        return false;
    }
    return true;
}

void Semaphore::post() {
    if (::sem_post(&semaphore_)) {
        throw errors::SemaphorePostError("semaphore post error");
    }
}

Semaphore::~Semaphore() noexcept {
    ::sem_destroy(&semaphore_);
}

}
