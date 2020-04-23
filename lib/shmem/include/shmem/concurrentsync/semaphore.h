#ifndef SHMEM_SHMEM_CONCURRENTSYNC_SEMAPHORE_H
#define SHMEM_SHMEM_CONCURRENTSYNC_SEMAPHORE_H

extern "C" {
#include <semaphore.h>
}

namespace shmem::concurrentsync {

class Semaphore {
  public:
    Semaphore(bool is_process_semaphore, unsigned int value);

    Semaphore(const Semaphore &) = delete;

    Semaphore &operator=(const Semaphore &) = delete;

    Semaphore(Semaphore &&other) noexcept;

    Semaphore &operator=(Semaphore && other) noexcept;

    void swap(Semaphore &other) noexcept;

    void wait();

    bool try_wait();

    void post();

    ~Semaphore() noexcept;

  private:
    sem_t semaphore_{};
    bool is_empty_ = true;

    Semaphore() noexcept = default;
};

}

#endif //SHMEM_SHMEM_CONCURRENTSYNC_SEMAPHORE_H
