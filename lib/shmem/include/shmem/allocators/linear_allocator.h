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

    explicit LinearAllocator(void *mmap_address, size_t mmap_size);

    template<typename NewT>
    explicit LinearAllocator(const LinearAllocator<NewT> &other) noexcept
            : shared_state_(other.shared_state_) {}

    template<typename NewT>
    LinearAllocator &operator=(const LinearAllocator<NewT> &other) noexcept {
        shared_state_ = other.shared_state_;
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
    }

    T *allocate(size_t size);

    void deallocate(T *ptr, size_t size);

    ~LinearAllocator() noexcept;

  private:
    using byte_type = std::byte;
    using byte_ptr_type = byte_type *;

    struct SharedState {
        void *start_shmem_address = nullptr;
        void *end_shmem_address = nullptr;
        void *free_shmem_address = nullptr;
    };

    SharedState *shared_state_ = nullptr;

    LinearAllocator() noexcept = default;
};


template<typename T>
LinearAllocator<T>::LinearAllocator(void *mmap_address, size_t mmap_size) {
    size_t init_used_mem_size = sizeof(SharedState) + sizeof(concurrentsync::Mutex);
    auto mmap_byte_addr = static_cast<byte_ptr_type>(mmap_address);

    if (mmap_byte_addr + init_used_mem_size > mmap_byte_addr + mmap_size) {
        throw std::bad_alloc();
    }

    shared_state_ = new(static_cast<SharedState *>(mmap_address)) SharedState;

    void *sem_addr = static_cast<SharedState *>(mmap_address) + 1;
    shared_state_->start_shmem_address = static_cast<concurrentsync::Mutex *>(sem_addr) + 1;

    shared_state_->end_shmem_address =
            static_cast<byte_ptr_type>(mmap_address) + mmap_size;
    shared_state_->free_shmem_address = shared_state_->start_shmem_address;
}

template<typename T>
T *LinearAllocator<T>::allocate(size_t size) {
    if (size == 0) {
        return nullptr;
    }

    void *old_free_shmem_addr = shared_state_->free_shmem_address;
    void *new_free_mem_addr =
            static_cast<byte_ptr_type>(shared_state_->free_shmem_address) + size;

    if (new_free_mem_addr > shared_state_->end_shmem_address) {
        throw std::bad_alloc();
    }

    shared_state_->free_shmem_address = new_free_mem_addr;

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

    void *old_free_shmem_addr = shared_state_->free_shmem_address;
    void *new_free_mem_addr =
            static_cast<byte_ptr_type>(shared_state_->free_shmem_address) - size;

    if (ptr == old_free_shmem_addr) {
        shared_state_->free_shmem_address = new_free_mem_addr;
    }
}

template<typename T>
LinearAllocator<T>::~LinearAllocator() noexcept {
    if (shared_state_ != nullptr) {
        shared_state_->~SharedState();
    }
}

}

#endif //SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H
