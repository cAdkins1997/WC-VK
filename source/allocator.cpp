
#include "allocator.h"
void exit_walker(void* ptr, size_t size, int used, void* user) {
    auto stats = static_cast<wcvk::allocators::MemoryStatistics*>(user);
    stats->add(used ? size : 0);

    if (used)
        std::cout << "Found active allocation" <<  ptr << ", " << size << '\n';
}

wcvk::allocators::HeapAllocator::~HeapAllocator() = default;

void wcvk::allocators::HeapAllocator::init(size_t size) {
    memory = malloc( size );
    max_size = size;
    allocated_size = 0;

    tlsf_handle = tlsf_create_with_pool(memory, size);

    printf( "HeapAllocator of size %llu created\n", size );
}

void wcvk::allocators::HeapAllocator::shutdown() {
    MemoryStatistics stats{0, max_size};
    pool_t pool = tlsf_get_pool(tlsf_handle);
    tlsf_walk_pool(pool, exit_walker, static_cast<void*>(&stats));

    if ( stats.allocated_bytes ) {
        std::cout << "HeapAllocator Shutdown.\n===============\nFAILURE! Allocated memory detected. allocated: " << stats.allocated_bytes << "total %llu\n===============\n\n";
    } else {
        printf("HeapAllocator Shutdown - all memory free!\n");
    }

    assert(stats.allocated_bytes == 0 && "Allocations still present. Check your code!");

    tlsf_destroy(tlsf_handle);
    free(memory);
}

void* wcvk::allocators::HeapAllocator::allocate(size_t size, size_t alignment) {
    return tlsf_malloc(tlsf_handle, size);
}

void * wcvk::allocators::HeapAllocator::allocate(size_t size, size_t alignment, const char *file, int32_t line) {
    return allocate(size, alignment);

}

void wcvk::allocators::HeapAllocator::deallocate(void *pointer) {
    tlsf_free(tlsf_handle, pointer);
}

wcvk::allocators::LinearAllocator::~LinearAllocator() = default;

void wcvk::allocators::LinearAllocator::init(const size_t size) {
    memory = static_cast<uint8_t*>(malloc(size));
    total_size = size;
    allocated_size = 0;
}

void wcvk::allocators::LinearAllocator::shutdown() {
    clear();
    free(memory);
}

void* wcvk::allocators::LinearAllocator::allocate(size_t size, size_t alignment) {
    assert(size > 0 && "tried to allocate using empty allocator");

    const size_t new_start = memory::memory_align( allocated_size, alignment );
    assert(new_start < total_size);

    const size_t new_allocated_size = new_start + size;

    if (new_allocated_size > total_size) {
        return nullptr;
    }

    allocated_size = new_allocated_size;
    return memory + new_start;
}

void* wcvk::allocators::LinearAllocator::allocate(size_t size, size_t alignment, const char* file, int32_t line) {
    return allocate(size, alignment);
}

void wcvk::allocators::LinearAllocator::deallocate(void *pointer) {
}

void wcvk::allocators::LinearAllocator::clear() {
    allocated_size = 0;
}

static wcvk::allocators::MemoryService s_memory_service;
wcvk::allocators::MemoryService* wcvk::allocators::MemoryService::instance() {
    return &s_memory_service;
}

void wcvk::allocators::MemoryService::init(void *configuration) {
    printf("Memory Service Init\n");
    auto memory_configuration = static_cast< MemoryServiceConfiguration*>( configuration );
    system_allocator.init(memory_configuration ? memory_configuration->maximum_dynamic_size : s_size);
}

void wcvk::allocators::MemoryService::shutdown() {
    system_allocator.shutdown();

    printf( "Memory Service Shutdown\n" );
}
