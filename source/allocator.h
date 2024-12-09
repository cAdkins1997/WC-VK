#pragma once
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <EASTL/memory.h>
#include "memory.h"
#include "../include/tlsf.h"

#include "service.h"

namespace wcvk::allocators {

    static size_t s_size = megabyte(32) + tlsf_size() + 8;

    struct MemoryStatistics {
        size_t allocated_bytes;
        size_t total_bytes;

        uint32_t allocation_count;

        void add(size_t allocation) {
            if (allocation) {
                allocated_bytes += allocation;
                ++allocation_count;
            }
        }
    };

    struct Allocator {
        virtual ~Allocator() = default;
        virtual void* allocate(size_t size, size_t alignment ) = 0;
        virtual void* allocate(size_t size, size_t alignment, const char* file, int32_t line ) = 0;
        virtual void deallocate( void* pointer ) = 0;
    };

    struct HeapAllocator final : Allocator {

        ~HeapAllocator() override;

        void init(size_t size );
        void shutdown();

        void debug_ui();

        void* allocate(size_t size, size_t alignment) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;

        void* tlsf_handle;
        void* memory;
        size_t allocated_size = 0;
        size_t max_size = 0;
    };

    struct StackAllocator final : Allocator {

        void init(size_t size );
        void shutdown();

        void* allocate(size_t size, size_t alignment ) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line) override;

        void deallocate(void* pointer) override;

        size_t get_marker();
        void free_marker(size_t marker);

        void clear();

        uint8_t* memory = nullptr;
        size_t total_size = 0;
        size_t allocated_size = 0;
    };

    struct LinearAllocator final : Allocator {

        ~LinearAllocator() override;

        void init(size_t size );
        void shutdown();

        void* allocate(size_t size, size_t alignment ) override;
        void* allocate(size_t size, size_t alignment, const char* file, int32_t line ) override;

        void deallocate(void* pointer) override;

        void clear();

        uint8_t* memory = nullptr;
        size_t total_size = 0;
        size_t allocated_size = 0;
    };

    struct MemoryServiceConfiguration {
        size_t maximum_dynamic_size = 32 * 1024 * 1024;
    };

    struct MemoryService final : Service {

        DECLARE_SERVICE(MemoryService);

        void init(void* configuration) override;
        void shutdown() override;

        LinearAllocator scratch_allocator;
        HeapAllocator system_allocator;

        static constexpr auto name = "memory service";
    };
}