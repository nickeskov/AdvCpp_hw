#ifndef SHMEM_SHMEM_CONTAINERS_MAP_H
#define SHMEM_SHMEM_CONTAINERS_MAP_H

#include <map>
#include <memory>
#include <functional>
#include <type_traits>
#include <mutex>

#include "shmem/allocators/linear_allocator.h"
#include "concurrentsync/mutex.h"
#include "concurrentsync/errors.h"
#include "raiiguards/scope_guard.h"

namespace shmem::containers {

template<typename Key,
        typename T,
        typename Compare = std::less<Key>,
        typename Allocator = shmem::allocators::LinearAllocator<std::pair<const Key, T>>,
        typename = typename std::allocator_traits<Allocator>::allocator_type>
class Map {
  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using allocator_type = Allocator;
    using key_compare = Compare;

    explicit Map(const Allocator &allocator) {
        concurrentsync::Mutex *mutex_mem;
        constexpr size_t mutex_size = sizeof(concurrentsync::Mutex);

        mutex_mem = static_cast<concurrentsync::Mutex *>(
                std::allocator_traits<Allocator>::allocate(
                        allocator, mutex_size
                )
        );

        std::map<Key, T, Compare, Allocator> *map_mem;
        constexpr size_t map_size = sizeof(std::map<Key, T, Compare, Allocator>);

        try {
            map_mem = static_cast<std::map<Key, T, Compare, Allocator> *>(
                    std::allocator_traits<Allocator>::allocate(
                            allocator, map_size
                    )
            );
        } catch (const std::bad_alloc &) {
            allocator->deallocate(mutex_mem, mutex_size);
            throw;
        }

        try {
            mutex_ptr_ = new(mutex_mem) concurrentsync::Mutex(true);
        } catch (...) {
            allocator->deallocate(map_mem, map_size);
            allocator->deallocate(mutex_mem, mutex_size);
            throw;
        }
        try {
            map_ptr_ = new(map_mem) std::map<Key, T, Compare, Allocator>(allocator);
        } catch (...) {
            mutex_ptr_->~Mutex();
            allocator->deallocate(map_mem, map_size);
            allocator->deallocate(mutex_mem, mutex_size);
        }
    }

    Map(const Map &) = delete;

    Map &operator=(const Map &) = delete;

    Map(Map &&other) noexcept {
        swap(other);
    }

    Map &operator=(Map &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        Map().swap(*this);
        swap(other);
        return *this;
    }

    void swap(Map &other) noexcept {
        std::swap(mutex_ptr_, other.mutex_ptr_);
        std::map(map_ptr_, other.map_ptr_);
    }

    allocator_type get_allocator() const noexcept {
        return map_ptr_->get_allocator();
    }

    T at(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
        T value = map_ptr_->at(key);
        return value;
    }

    auto extract(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
        mutex_ptr_->lock();
        return map_ptr_->extract(key);
    }

    template<std::enable_if<std::is_trivially_copyable_v<T>, void> *>
    void emplace(const value_type &value) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
    }

    ~Map() noexcept {
        if (map_ptr_ != nullptr && mutex_ptr_ != nullptr) {
            map_ptr_->clear();
            map_ptr_->~map();
            mutex_ptr_->~Mutex();
        }
    }

  private:
    concurrentsync::Mutex *mutex_ptr_ = nullptr;
    std::map<Key, T, Compare, Allocator> *map_ptr_ = nullptr;

    Map() noexcept = default;
};

}

#endif //SHMEM_SHMEM_CONTAINERS_MAP_H
