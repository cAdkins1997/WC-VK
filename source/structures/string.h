#pragma once
#include "../allocator.h"

namespace wcvk {

    struct StringView {
        char* data{};
        size_t length{};

        static bool equals(const StringView& a, const StringView& b);
        static void copy(const StringView& a, char* buffer, size_t bufferSize);
    };

    struct StringBuffer {
        void init(size_t size, allocators::Allocator& allocator);
        void shutdown();

        void append(const char* string);
        void append(const StringView& text);
        void append_memory(void* memory, size_t size);
        void append(const StringBuffer& other_buffer);
        void append_format( const char* format, ...);
        char* append_use(const char* string);
        char* append_use_format(const char* format, ...);
        char* append_use(const StringView& text);
        char* append_use_substring( const char* string, uint32_t start_index, uint32_t end_index );

        void close_current_string();

        uint32_t get_index(const char* text ) const;
        [[nodiscard]] char* get_text(uint32_t index) const;
        char* reserve(size_t size);
        char* current(){ return data + current_size; }
        void clear();
        char* data;
        uint32_t buffer_size = 1024;
        uint32_t current_size = 0;
        allocators::Allocator* allocator_ = nullptr;
    };
}
