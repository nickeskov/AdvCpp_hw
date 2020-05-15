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
    using value_type = std::pair<const key_type, mapped_type>;
    using allocator_type = Allocator;
    using key_compare = Compare;
    using size_type = size_t;

    explicit Map(Allocator &allocator, bool is_interprocess_map)
            : mutex_allocator_ptr_(std::make_unique<mutex_allocator_type>(allocator)),
              map_allocator_ptr_(std::make_unique<map_allocator_type>(allocator)) {

        mutex_type *mutex_mem;

        mutex_mem = std::allocator_traits<mutex_allocator_type>::allocate(
                *mutex_allocator_ptr_, 1
        );

        // cppcheck-suppress variableScope symbolName=map_mem // it's used below
        map_type *map_mem;

        try {
            // cppcheck-suppress unreadVariable symbolName=map_mem // it's used below
            map_mem = std::allocator_traits<map_allocator_type>::allocate(
                    *map_allocator_ptr_, 1
            );
        } catch (...) {
            std::allocator_traits<mutex_allocator_type>::deallocate(
                    *mutex_allocator_ptr_, mutex_mem, 1
            );
            throw;
        }

        try {
            mutex_ptr_ = new(mutex_mem) mutex_type{is_interprocess_map};
        } catch (...) {
            std::allocator_traits<map_allocator_type>::deallocate(
                    *map_allocator_ptr_, map_mem, 1
            );
            std::allocator_traits<mutex_allocator_type>::deallocate(
                    *mutex_allocator_ptr_, mutex_mem, 1
            );
            throw;
        }
        try {
            map_ptr_ = new(map_mem) map_type{allocator};
        } catch (...) {
            mutex_ptr_->~Mutex();
            std::allocator_traits<map_allocator_type>::deallocate(
                    *map_allocator_ptr_, map_mem, 1
            );
            std::allocator_traits<mutex_allocator_type>::deallocate(
                    *mutex_allocator_ptr_, mutex_mem, 1
            );
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
        std::swap(map_ptr_, other.map_ptr_);
        std::swap(mutex_allocator_ptr_, other.mutex_allocator_ptr_);
        std::swap(map_allocator_ptr_, other.map_allocator_ptr_);
    }

    auto get_allocator() const noexcept {
        return map_ptr_->get_allocator();
    }

    mapped_type at(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->at(key);
    }

    template<typename RetT = mapped_type, typename NewKey,
            typename = std::enable_if_t<std::is_constructible_v<RetT, mapped_type>>>
    RetT at(const NewKey &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);

        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        auto node_value = map_ptr_->at(node_key);

        RetT value{node_value};
        return value;
    }

    template<typename NewKey, typename NewT>
    void insert(const NewKey &key, const NewT &value) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);

        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        auto node_value = get_obj_copy<mapped_type, NewT, allocator_type>(value);

        map_ptr_->emplace(std::move(node_key), std::move(node_value));
    }

    template<typename NewKey, typename NewT,
            std::enable_if<std::is_assignable_v<mapped_type, NewT>> * = nullptr>
    void insert_or_assign(const NewKey &key, NewT &&value) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);

        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);

        map_ptr_->insert_or_assign(node_key, std::forward<NewT>(value));
    }

    template<typename NewKey, typename NewT,
            std::enable_if<std::is_constructible_v<mapped_type, NewT>> * = nullptr>
    void insert_or_assign(const NewKey &key, const NewT &value) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);

        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        auto node_value = get_obj_copy<mapped_type, NewT, allocator_type>(value);

        map_ptr_->insert_or_assign(node_key, node_value);
    }

    size_type erase(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->erase(key);
    }

    template<typename NewKey>
    size_type erase(const NewKey &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        return map_ptr_->erase(node_key);
    }

    auto extract(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->extract(key);
    }

    template<typename NewKey>
    auto extract(const NewKey &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        return map_ptr_->extract(node_key);
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

    template<typename NewKey>
    bool contains(const NewKey &key) const {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);

        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);

        return map_ptr_->find(key) != map_ptr_->cend();
    }

    void destroy() noexcept {
        if (map_ptr_ != nullptr && mutex_ptr_ != nullptr) {
            map_ptr_->clear();
            map_ptr_->~map();
            mutex_ptr_->~Mutex();

            std::allocator_traits<map_allocator_type>::deallocate(
                    *map_allocator_ptr_, map_ptr_, 1
            );
            std::allocator_traits<mutex_allocator_type>::deallocate(
                    *mutex_allocator_ptr_, mutex_ptr_, 1
            );
        }
    }

    ~Map() noexcept {
        map_ptr_ = nullptr;
        map_allocator_ptr_ = nullptr;

        mutex_ptr_ = nullptr;
        mutex_allocator_ptr_ = nullptr;
    }

  private:
    using mutex_type = concurrentsync::Mutex;
    using map_type = std::map<key_type, mapped_type, Compare, Allocator>;

    using mutex_allocator_type = typename std::allocator_traits<Allocator>::
    template rebind_alloc<mutex_type>;

    using map_allocator_type = typename std::allocator_traits<Allocator>::
    template rebind_alloc<map_type>;

    mutable concurrentsync::Mutex *mutex_ptr_ = nullptr;
    map_type *map_ptr_ = nullptr;

    std::unique_ptr<mutex_allocator_type> mutex_allocator_ptr_;
    std::unique_ptr<map_allocator_type> map_allocator_ptr_;

    Map() noexcept = default;

    template<typename RetT, typename Type, typename AllocT = allocator_type,
            std::enable_if_t<
                    std::uses_allocator_v<RetT, AllocT> &&
                    std::is_copy_assignable_v<RetT> &&
                    std::is_assignable_v<RetT, Type> &&
                    std::is_constructible_v<RetT, AllocT>, void> * = nullptr>
    RetT get_obj_copy(const Type &obj) const {
        RetT new_obj{get_allocator()};
        new_obj = obj;
        return new_obj;
    }

    template<typename RetT, typename Type, typename AllocT = allocator_type,
            std::enable_if_t<
                    std::is_pod_v<RetT> &&
                    std::is_pod_v<Type>, void> * = nullptr>
    RetT get_obj_copy(const Type &obj) const {
        return {obj};
    }
};

}

#endif //SHMEM_SHMEM_CONTAINERS_MAP_H
