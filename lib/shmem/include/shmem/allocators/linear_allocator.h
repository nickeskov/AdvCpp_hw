#ifndef SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H
#define SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H

#include <memory>
#include <cinttypes>
#include <cstddef>

#include "shmem/errors.h"

namespace shmem::allocators {

struct LinearAllocatorSharedState {
  public:
    void *start_shmem_address = nullptr;
    void *end_shmem_address = nullptr;
    void *free_shmem_address = nullptr;
};

template<typename T>
class LinearAllocator {
  public:
    using value_type = T;
    using size_type = size_t;

    template<typename NewT>
    struct rebind {
        using other = LinearAllocator<NewT>;
    };

    explicit LinearAllocator(void *mmap_address, size_type mmap_size);

    template<typename NewT>
    LinearAllocator(const LinearAllocator<NewT> &other) noexcept // NOLINT explicit
            : shared_state_(other.shared_state_) {}

    template<typename NewT>
    LinearAllocator &operator=(const LinearAllocator<NewT> &other) noexcept {
        shared_state_ = other.shared_state_;
        return *this;
    }

    template<typename NewT>
    LinearAllocator(LinearAllocator<NewT> &&other) noexcept { // NOLINT explicit
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
    void swap(LinearAllocator<NewT> &other) noexcept {
        std::swap(shared_state_, other.shared_state_);
    }

    T *allocate(size_type size);

    void deallocate(T *ptr, size_type size);

    ~LinearAllocator() noexcept;

  public:
    using byte_type = std::byte;

    LinearAllocatorSharedState *shared_state_ = nullptr;

  private:
    LinearAllocator() noexcept = default;
};

template<typename T>
LinearAllocator<T>::LinearAllocator(void *mmap_address, size_type mmap_size) {
    constexpr size_type init_used_mem_size = sizeof(LinearAllocatorSharedState);
    auto mmap_byte_addr = static_cast<byte_type *>(mmap_address);

    if (mmap_byte_addr + init_used_mem_size > mmap_byte_addr + mmap_size) {
        throw std::bad_alloc();
    }

    shared_state_ = new(static_cast<LinearAllocatorSharedState *>(mmap_address))
            LinearAllocatorSharedState;

    shared_state_->start_shmem_address =
            static_cast<LinearAllocatorSharedState *>(mmap_address) + 1;

    shared_state_->end_shmem_address =
            static_cast<byte_type *>(mmap_address) + mmap_size;
    shared_state_->free_shmem_address = shared_state_->start_shmem_address;
}

template<typename T>
T *LinearAllocator<T>::allocate(size_type size) {
    if (size == 0) {
        return nullptr;
    }

    void *new_free_mem_addr =
            static_cast<byte_type *>(shared_state_->free_shmem_address) + size * sizeof(T);

    if (new_free_mem_addr > shared_state_->end_shmem_address) {
        throw std::bad_alloc();
    }

    void *old_free_shmem_addr = shared_state_->free_shmem_address;

    shared_state_->free_shmem_address = new_free_mem_addr;

    return static_cast<T *>(old_free_shmem_addr);
}

template<typename T>
void LinearAllocator<T>::deallocate(T *ptr, size_type size) {
    if (ptr == nullptr) {
        return;
    }

    if (ptr > shared_state_->end_shmem_address || ptr < shared_state_->start_shmem_address) {
        throw errors::DeallocateError(
                "invalid address for deallocate shared memory");
    }

    void *old_free_shmem_addr = shared_state_->free_shmem_address;

    if (ptr == old_free_shmem_addr) {
        void *new_free_mem_addr =
                static_cast<byte_type *>(shared_state_->free_shmem_address) - size * sizeof(T);
        shared_state_->free_shmem_address = new_free_mem_addr;
    }
}

template<typename T>
LinearAllocator<T>::~LinearAllocator() noexcept {
    if (shared_state_ != nullptr) {
        shared_state_->~LinearAllocatorSharedState();
    }
}

}

#endif //SHMEM_SHMEM_ALLOCATORS_LINEAR_ALLOCATOR_H
