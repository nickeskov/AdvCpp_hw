#ifndef SHMEM_SHMEM_TYPES_H
#define SHMEM_SHMEM_TYPES_H

#include <string>

#include "shmem/allocators/linear_allocator.h"

namespace shmem::types {

template<typename Allocator = shmem::allocators::LinearAllocator<char>>
using string = std::basic_string<char, std::char_traits<char>, Allocator>;

template<typename Allocator = shmem::allocators::LinearAllocator<wchar_t>>
using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>;

}

#endif //SHMEM_SHMEM_TYPES_H
