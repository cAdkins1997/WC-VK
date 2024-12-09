
#ifndef MEMORY_H
#define MEMORY_H
#include <cstring>

namespace wcvk::memory {
#define kilobyte(size) (size * 1024)
#define megabyte(size) (size * 1024 * 1024)
#define gigabyte(size) (size * 1024 * 1024 * 1024)

    void memory_copy( void* destination, void* source, size_t size );

    size_t memory_align(size_t size, size_t alignment );
}

#endif //MEMORY_H
