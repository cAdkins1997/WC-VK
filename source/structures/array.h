#pragma once
#include "../allocator.h"

namespace wcvk {
    template<typename T>
    class Array {
    public:
        Array() = default;

        ~Array() {
            assert(data != nullptr);
        }

        void init(allocators::Allocator* allocator_, const size_t initial_capacity, const size_t initial_size = 0) {
            data = nullptr;
            size = initial_size;
            capacity = 0;
            allocator = allocator_;

            if (initial_capacity > 0) {
                grow(initial_capacity);
            }
        }

        void shutdown() {
            if (capacity > 0) {
                allocator->deallocate(data);
            }
            data = nullptr;
            size = capacity = 0;
        }

        void push(const T& element) {
            if (size >= capacity) {
                grow(capacity + 1);
            }
            data[size++] = element;
        }

        T& push_use() {
            if ( size >= capacity ) {
                grow( capacity + 1 );
            }
            ++size;

            return back();
        }

        void pop() {
            assert( size > 0 );
            --size;
        }

        void delete_swap(size_t index ) {
            assert(size > 0 && index < size);
            data[index] = data[--size];
        }

        T& operator [] (size_t index) {
            assert(index < size);
            return data[index];
        }

        const T& operator [] (size_t index) const {
            assert(index < size);
            return data[index];
        }

        void clear() {
            size = 0;
        }

        void set_size(size_t new_size ) {
            if (new_size > capacity) {
                grow(new_size);
            }
            size = new_size;
        }

        void set_capacity(size_t new_capacity) {
            if (new_capacity > capacity) {
                grow(new_capacity);
            }
        }

        void grow(size_t new_capacity) {
            if ( new_capacity < capacity * 2 ) {
                new_capacity = capacity * 2;
            } else if ( new_capacity < 4 ) {
                new_capacity = 4;
            }

            T* new_data = static_cast<T*>(allocator->allocate(new_capacity * sizeof(T), alignof(T)));
            if ( capacity ) {
                memory::memory_copy(new_data, data, capacity * sizeof( T ));

                allocator->deallocate( data );
            }

            data = new_data;
            capacity = new_capacity;
        }

        T& back() {
            return data[size - 1];
        }

        const T& back() const {
            return data[size - 1];
        }

        T& front() {
            return data[0];
        }
        const T& front() const {
            return data[0];
        }

        [[nodiscard]] uint32_t size_in_bytes() const {
            return size * sizeof(T);
        }
        [[nodiscard]] uint32_t capacity_in_bytes() const {
            return capacity * sizeof(T);
        }


        T* data;
        size_t size{};
        size_t capacity{};
        allocators::Allocator* allocator{};
    };
}

