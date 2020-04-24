#ifndef SHMEM_SHMEM_SHARED_MEMORY_H
#define SHMEM_SHMEM_SHARED_MEMORY_H

#include <memory>
#include <functional>

extern "C" {
#include <sys/mman.h>
}

namespace shmem {

template<typename T>
using shmem_unique_ptr = std::unique_ptr<T, std::function<void(T *)>>;

template<typename T>
shmem_unique_ptr<T> create_shmem(size_t size) {
    void *mmap_ptr = ::mmap(nullptr,
                            sizeof(T) * size,
                            PROT_WRITE | PROT_READ, // NOLINT this is system values, it's valid
                            MAP_SHARED | MAP_ANONYMOUS, // NOLINT this is system values, it's valid
                            -1,
                            0);

    if (mmap_ptr == MAP_FAILED) {
        throw std::bad_alloc();
    }

    return {
            static_cast<T *>(mmap_ptr),
            [size](T *mmap_addr) {
                ::munmap(mmap_addr, size);
            }
    };
}

}

#endif //SHMEM_SHMEM_SHARED_MEMORY_H
