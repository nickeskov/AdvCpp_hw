#ifndef CONCURRENTSYNC_CONCURRENTSYNC_MUTEX_H
#define CONCURRENTSYNC_CONCURRENTSYNC_MUTEX_H

extern "C" {
#include "semaphore.h"
}

namespace concurrentsync {

class Mutex {
  public:
    explicit Mutex(bool is_process_mutex);

    Mutex(const Mutex &) = delete;

    Mutex &operator=(const Mutex &) = delete;

    Mutex(Mutex && other) noexcept;

    Mutex &operator=(Mutex &&other) noexcept ;

    void swap(Mutex &other) noexcept;

    void lock();

    bool try_lock();

    void unlock();

    ~Mutex() noexcept;

  private:
    sem_t mutex_{};
    bool is_empty_ = true;

    Mutex() noexcept = default;
};

}

#endif //CONCURRENTSYNC_CONCURRENTSYNC_MUTEX_H
