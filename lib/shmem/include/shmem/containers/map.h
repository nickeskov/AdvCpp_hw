#ifndef SHMEM_SHMEM_CONTAINERS_MAP_H
#define SHMEM_SHMEM_CONTAINERS_MAP_H

#include <map>
#include <memory>
#include <functional>
#include <type_traits>
#include <mutex>

#include "shmem/allocators/linear_allocator.h"
#include "shmem/concurrentsync/mutex.h"
#include "shmem/errors.h"

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
    using size_type = size_t;

    explicit Map(Allocator &allocator, bool is_interprocess_map) {
        using alloc_ptr_t = typename std::allocator_traits<Allocator>::pointer;

        concurrentsync::Mutex *mutex_mem;
        constexpr size_t mutex_size = sizeof(concurrentsync::Mutex);

        mutex_mem = reinterpret_cast<concurrentsync::Mutex *>(
                std::allocator_traits<Allocator>::allocate(
                        allocator, mutex_size
                )
        );

        std::map<Key, T, Compare, Allocator> *map_mem;
        constexpr size_t map_size = sizeof(std::map<Key, T, Compare, Allocator>);

        try {
            map_mem = reinterpret_cast<std::map<Key, T, Compare, Allocator> *>(
                    std::allocator_traits<Allocator>::allocate(
                            allocator, map_size
                    )
            );
        } catch (...) {
            std::allocator_traits<Allocator>::deallocate(allocator,
                                                         reinterpret_cast<alloc_ptr_t>(mutex_mem),
                                                         mutex_size);
            throw;
        }

        try {
            mutex_ptr_ = new(mutex_mem) concurrentsync::Mutex{is_interprocess_map};
        } catch (...) {
            std::allocator_traits<Allocator>::deallocate(allocator,
                                                         reinterpret_cast<alloc_ptr_t>(map_mem),
                                                         map_size);
            std::allocator_traits<Allocator>::deallocate(allocator,
                                                         reinterpret_cast<alloc_ptr_t>(mutex_mem),
                                                         mutex_size);
            throw;
        }
        try {
            map_ptr_ = new(map_mem) std::map<Key, T, Compare, Allocator>{allocator};
        } catch (...) {
            mutex_ptr_->~Mutex();
            std::allocator_traits<Allocator>::deallocate(allocator,
                                                         reinterpret_cast<alloc_ptr_t>(map_mem),
                                                         map_size);
            std::allocator_traits<Allocator>::deallocate(allocator,
                                                         reinterpret_cast<alloc_ptr_t>(mutex_mem),
                                                         mutex_size);
            throw;
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

    T at(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        T value = get_obj_copy(map_ptr_->at(key));
        return value;
    }

    auto extract(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        mutex_ptr_->lock();
        return map_ptr_->extract(key);
    }

    void insert(const value_type &value) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        auto node_key = get_obj_copy(value.first);
        auto node_value = get_obj_copy(value.second);
        map_ptr_->emplace(std::make_pair(std::move(node_key), std::move(node_value)));
    }

    size_type erase(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->erase(key);
    }

    void clear() {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        map_ptr_->clear();
    }

    bool empty() const {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->empty();
    }

    size_type size() const {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->size();
    }

    size_type count(const key_type &key) const {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->count(key);
    }

    bool contains(const key_type &key) const {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->find(key) != map_ptr_->cend();
    }

    ~Map() noexcept {
        if (map_ptr_ != nullptr && mutex_ptr_ != nullptr) {
            map_ptr_->clear();
            map_ptr_->~map();
            mutex_ptr_->~Mutex();
        }
    }

  private:
    mutable concurrentsync::Mutex *mutex_ptr_ = nullptr;
    std::map<Key, T, Compare, Allocator> *map_ptr_ = nullptr;

    Map() noexcept = default;

    template<typename Type, typename AllocT = allocator_type,
            std::enable_if_t<std::is_pod_v<std::decay<Type>>, void> * = nullptr>
    auto get_obj_copy(Type &&ojb) const {
        return std::forward<Type>(ojb);
    }

    template<typename Type, typename AllocT = allocator_type,
            std::enable_if_t<std::uses_allocator_v<std::decay<Type>, AllocT>
                             && std::is_copy_assignable_v<std::decay<Type>>
                             && std::is_constructible_v<std::decay<Type>, AllocT>, void> * = nullptr>
    auto get_obj_copy(Type &&ojb) const {
        auto new_obj{get_allocator()};
        new_obj = std::forward<Type>(ojb);
        return new_obj;
    }
};

}

#endif //SHMEM_SHMEM_CONTAINERS_MAP_H
