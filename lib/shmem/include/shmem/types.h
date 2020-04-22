#ifndef SHMEM_SHMEM_TYPES_H
#define SHMEM_SHMEM_TYPES_H

#include <string>

#include "shmem/allocators/linear_allocator.h"

namespace shmem::types {

using string = std::basic_string<char, std::char_traits<char>, shmem::allocators::LinearAllocator<char>);

using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, shmem::allocators::LinearAllocator<wchar_t>>

}

#endif //SHMEM_SHMEM_TYPES_H
