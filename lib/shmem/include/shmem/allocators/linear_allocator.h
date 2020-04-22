#ifndef SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H
#define SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H

#include <memory>
#include <atomic>
#include <shared_mutex>
#include <cinttypes>
#include <cstddef>

extern "C" {
#include <sys/mman.h>
}

#include "shmem/errors.h"
#include "concurrentsync/mutex.h"
#include "concurrentsync/errors.h"

namespace shmem::allocators {

template<typename T>
class LinearAllocator {
  public:
    using value_type = T;

    explicit LinearAllocator(size_t shmem_size);

    template<typename NewT>
    explicit LinearAllocator(const LinearAllocator<NewT> &other) noexcept
            : shared_state_(other.shared_state_),
              local_reference_count_(other.local_reference_count_) {}

    template<typename NewT>
    LinearAllocator &operator=(const LinearAllocator<NewT> &other) noexcept {
        if (this == &other) {
            return *this;
        }
        shared_state_ = other.shared_state_;
        local_reference_count_ = other.local_reference_count_;
        return *this;
    }

    template<typename NewT>
    explicit LinearAllocator(LinearAllocator<NewT> &&other) noexcept {
        swap(other);
    }

    template<typename NewT>
    LinearAllocator &operator=(LinearAllocator<NewT> &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        LinearAllocator().swap(*this);
        swap(other);
        return *this;
    }

    template<typename NewT>
    void swap(const LinearAllocator<NewT> &other) noexcept {
        std::swap(shared_state_, other.shared_state_);
        std::swap(local_reference_count_, local_reference_count_);
    }

    T *allocate(size_t size);

    void deallocate(T *ptr, size_t size);

    ~LinearAllocator() noexcept;

  private:
    using byte_type = std::byte;
    using byte_ptr_type = byte_type *;

    struct SharedState {
        concurrentsync::Mutex *mutex_ptr = nullptr;
        ssize_t total_reference_count = -1;

        void *mmap_address = nullptr;

        void *start_shmem_address = nullptr;
        void *end_shmem_address = nullptr;
        void *free_shmem_address = nullptr;
    };

    SharedState *shared_state_ = nullptr;
    std::shared_ptr<ssize_t> local_reference_count_;

    LinearAllocator() noexcept = default;
};


template<typename T>
LinearAllocator<T>::LinearAllocator(size_t shmem_size) {
    size_t total_shmem_size = shmem_size + sizeof(SharedState);
    void *mmap_address = mmap(nullptr,
                              total_shmem_size,
                              PROT_READ | PROT_WRITE, // NOLINT it is unsigned valid values
                              MAP_SHARED | MAP_ANONYMOUS, // NOLINT it is unsigned valid values
                              -1,
                              0);

    if (mmap_address == MAP_FAILED) {
        throw std::bad_alloc();
    }
    shared_state_ = new(static_cast<SharedState *>(mmap_address)) SharedState;
    shared_state_->mmap_address = mmap_address;

    try {
        void *sem_addr = static_cast<SharedState *>(mmap_address) + 1;
        shared_state_->mutex_ptr =
                new (static_cast<concurrentsync::Mutex *>(sem_addr)) concurrentsync::Mutex(true);
        shared_state_->start_shmem_address = static_cast<concurrentsync::Mutex *>(sem_addr) + 1;
    } catch (const concurrentsync::errors::MutexInitError &) {
        munmap(mmap_address, total_shmem_size);
        throw;
    }
    shared_state_->end_shmem_address =
            static_cast<byte_ptr_type>(mmap_address) + total_shmem_size;
    shared_state_->free_shmem_address = shared_state_->start_shmem_address;
    shared_state_->total_reference_count = 1;

    *local_reference_count_ = 1;
}

template<typename T>
T *LinearAllocator<T>::allocate(size_t size) {
    if (size == 0) {
        return nullptr;
    }

    shared_state_->mutex_ptr->lock();

    void *old_free_shmem_addr = shared_state_->free_shmem_address;
    void *new_free_mem_addr =
            static_cast<byte_ptr_type>(shared_state_->free_shmem_address) + size;

    if (new_free_mem_addr > shared_state_->end_shmem_address) {
        throw std::bad_alloc();
    }

    shared_state_->free_shmem_address = new_free_mem_addr;

    try {
        shared_state_->mutex_ptr->unlock();
    }  catch (const concurrentsync::errors::MutexUnlockError &) {
        shared_state_->free_shmem_address = old_free_shmem_addr;
        throw;
    }

    return static_cast<T *>(old_free_shmem_addr);
}

template<typename T>
void LinearAllocator<T>::deallocate(T *ptr, size_t size) {
    if (ptr == nullptr) {
        return;
    }

    if (ptr > shared_state_->end_shmem_address || ptr < shared_state_->start_shmem_address) {
        throw errors::DeallocateError(
                "invalid address for deallocate shared memory");
    }

    shared_state_->mutex_ptr->lock();

    void *old_free_shmem_addr = shared_state_->free_shmem_address;
    void *new_free_mem_addr =
            static_cast<byte_ptr_type>(shared_state_->free_shmem_address) - size;

    if (ptr == old_free_shmem_addr) {
        shared_state_->free_shmem_address = new_free_mem_addr;
    }

    try {
        shared_state_->mutex_ptr->unlock();
    }  catch (const concurrentsync::errors::MutexUnlockError &) {
        shared_state_->free_shmem_address = old_free_shmem_addr;
        throw;
    }
}

template<typename T>
LinearAllocator<T>::~LinearAllocator() noexcept {
    if (shared_state_ != nullptr) {
        // process lock
        try {
            shared_state_->mutex_ptr->wait();
        } catch (...) {}

        // decrement total reference counter and check if this last reference on shared memory
        if (--shared_state_->total_reference_count == 0) {
            shared_state_->mutex_ptr->~Semaphore();
            shared_state_->~SharedState();

            munmap(shared_state_->mmap_address, shared_state_->total_shmem_size);
            return;
        }
        // free process lock
        try {
            shared_state_->mutex_ptr->post();
        } catch (...) {}

        // thread-safety: not
        // decrement local process reference counter and perform cleanups
        if (local_reference_count_ && --(*local_reference_count_) == 0) {
            shared_state_->~SharedState();
            munmap(shared_state_->mmap_address, shared_state_->total_shmem_size);
        }
    }
}

}

#endif //SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H
