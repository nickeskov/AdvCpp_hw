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
        concurrentsync::Mutex * mutex_mem = nullptr;
        std::map<Key, T, Compare, Allocator> *map_mem = nullptr;

        constexpr size_t mutex_size = sizeof(concurrentsync::Mutex);
        constexpr size_t map_size = sizeof(std::map<Key, T, Compare, Allocator>);

        mutex_mem = static_cast<concurrentsync::Mutex *>(
                std::allocator_traits<Allocator>::allocate(
                        allocator, mutex_size
                )
        );

        try {
            map_mem = static_cast<std::map<Key, T, Compare, Allocator> *>(
                    std::allocator_traits<Allocator>::allocate(
                            allocator, map_size
                    )
            );
        } catch (const std::bad_alloc &) {
            allocator.deallocate(mutex_mem, mutex_size);
            throw;
        }

        try {
            mutex_ptr_ = new (mutex_mem) concurrentsync::Mutex(true);
        } catch (...) {
            allocator.deallocate(map_mem, map_size);
            allocator.deallocate(mutex_mem, mutex_size);
            throw;
        }
        try {
            map_ = new (map_mem) std::map<Key, T, Compare, Allocator>(allocator);
        } catch (...) {
            mutex_ptr_->~Mutex();
            allocator.deallocate(map_mem, map_size);
            allocator.deallocate(mutex_mem, mutex_size);
        }
    }

    T at(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
        T value = map_->at(key);
        return value;
    }

    auto extract(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
        mutex_ptr_->lock();
        return map_->extract(key);
    }

    template<std::enable_if<std::is_trivially_copyable_v<T>, void> *>
    void emplace(const value_type &value) {
        std::lock_guard<concurrentsync::Mutex> guard(*mutex_ptr_);
    }


  private:
    concurrentsync::Mutex *mutex_ptr_ = nullptr; // TODO free mem in destructor
    std::map<Key, T, Compare, Allocator> *map_ = nullptr; // TODO free mem in destructor
};

}

#endif //SHMEM_SHMEM_CONTAINERS_MAP_H
