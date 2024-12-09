
#include "memory.h"

void wcvk::memory::memory_copy(void *destination, void *source, size_t size) {
    memcpy(destination, source, size);
}

size_t wcvk::memory::memory_align(size_t size, size_t alignment) {
    const size_t alignment_mask = alignment - 1;
    return (size + alignment_mask) & ~alignment_mask;
}
