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

    explicit Map(Allocator &allocator, bool is_interprocess_map)
            : mutex_allocator_ptr_(std::make_unique<mutex_allocator_t>(allocator)),
              map_allocator_ptr_(std::make_unique<map_allocator_t>(allocator)) {

        concurrentsync::Mutex *mutex_mem;

        mutex_mem = std::allocator_traits<mutex_allocator_t>::allocate(
                *mutex_allocator_ptr_, 1
        );

        std::map<Key, T, Compare, Allocator> *map_mem;

        try {
            map_mem = std::allocator_traits<map_allocator_t>::allocate(
                    *map_allocator_ptr_, 1
            );
        } catch (...) {
            std::allocator_traits<mutex_allocator_t>::deallocate(
                    *mutex_allocator_ptr_, mutex_mem, 1
            );
            throw;
        }

        try {
            mutex_ptr_ = new(mutex_mem) concurrentsync::Mutex{is_interprocess_map};
        } catch (...) {
            std::allocator_traits<map_allocator_t>::deallocate(
                    *map_allocator_ptr_, map_mem, 1
            );
            std::allocator_traits<mutex_allocator_t>::deallocate(
                    *mutex_allocator_ptr_, mutex_mem, 1
            );
            throw;
        }
        try {
            map_ptr_ = new(map_mem) std::map<Key, T, Compare, Allocator>{allocator};
        } catch (...) {
            mutex_ptr_->~Mutex();
            std::allocator_traits<map_allocator_t>::deallocate(
                    *map_allocator_ptr_, map_mem, 1
            );
            std::allocator_traits<mutex_allocator_t>::deallocate(
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
        std::map(map_ptr_, other.map_ptr_);
        std::swap(mutex_allocator_ptr_, other.mutex_allocator_ptr_);
        std::swap(map_allocator_ptr_, other.map_allocator_ptr_);
    }

    allocator_type get_allocator() const noexcept {
        return map_ptr_->get_allocator();
    }

    T at(const Key &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        return map_ptr_->at(key);
    }

    template<typename NewKey, typename RetT = T>
    RetT at(const NewKey &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        auto obj = map_ptr_->at(node_key);
        RetT value = obj;
        return value;
    }

    auto extract(const key_type &key) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        mutex_ptr_->lock();
        return map_ptr_->extract(key);
    }

    template<typename NewKey, typename NewT>
    void insert(const NewKey &key, const NewT &value) {
        std::lock_guard<concurrentsync::Mutex> lock(*mutex_ptr_);
        auto node_key = get_obj_copy<key_type, NewKey, allocator_type>(key);
        auto node_value = get_obj_copy<T, NewT, allocator_type>(value);
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

    void destroy() noexcept {
        if (map_ptr_ != nullptr && mutex_ptr_ != nullptr) {
            map_ptr_->clear();
            map_ptr_->~map();
            mutex_ptr_->~Mutex();

            std::allocator_traits<map_allocator_t>::deallocate(
                    *map_allocator_ptr_, map_ptr_, 1
            );
            std::allocator_traits<mutex_allocator_t>::deallocate(
                    *mutex_allocator_ptr_, mutex_ptr_, 1
            );
        }
    }

    ~Map() noexcept = default;

  private:
    using mutex_t = concurrentsync::Mutex;
    using map_t = std::map<Key, T, Compare, Allocator>;

    using mutex_allocator_t = typename std::allocator_traits<Allocator>::
    template rebind_alloc<mutex_t>;

    using map_allocator_t = typename std::allocator_traits<Allocator>::
    template rebind_alloc<map_t>;

    mutable concurrentsync::Mutex *mutex_ptr_ = nullptr;
    map_t *map_ptr_ = nullptr;

    std::unique_ptr<mutex_allocator_t> mutex_allocator_ptr_;
    std::unique_ptr<map_allocator_t> map_allocator_ptr_;

    Map() noexcept = default;

//    // TODO its not working :(
//    template<typename RetT, typename Type, typename AllocT = allocator_type,
//            std::enable_if_t<
//                    std::uses_allocator_v<RetT, AllocT>
//                    &&
//                    std::is_copy_assignable_v<std::decay<Type>> &&
//                    std::is_constructible_v<std::decay_t<Type>, AllocT>
//                    , void> * = nullptr>
//    RetT get_obj_copy(const Type &ojb) const {
//        RetT new_obj{get_allocator()};
//        new_obj = ojb;
//        return new_obj;
//    }

    template<typename RetT, typename Type, typename AllocT = allocator_type,
            std::enable_if_t<
                    std::is_pod_v<RetT>
                    && std::is_constructible_v<RetT, Type>, void> * = nullptr>
    RetT get_obj_copy(const Type &ojb) const {
        return {ojb};
    }

};

}

#endif //SHMEM_SHMEM_CONTAINERS_MAP_H
