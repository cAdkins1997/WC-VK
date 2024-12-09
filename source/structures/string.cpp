#include "../structures/string.h"
#include <cstdarg>

namespace wcvk {
    bool StringView::equals(const StringView &a, const StringView &b) {
        if (a.length != b.length)
            return false;

        for (size_t i = 0; i < a.length; ++i) {
            if (a.data[i] != b.data[i]) {
                return false;
            }
        }

        return true;
    }

    void StringView::copy(const StringView &a, char *buffer, size_t bufferSize) {
        const size_t max_length = bufferSize - 1 < a.length ? bufferSize - 1 : a.length;
        memory::memory_copy(buffer, a.data, max_length);
        buffer[a.length] = 0;
    }

    void StringBuffer::init(const size_t size, allocators::Allocator& allocator) {
        if (data) {
            allocator.deallocate(data);
        }

        if (size < 1) {
            printf("ERROR: Buffer cannot be empty!\n");
            return;
        }
        allocator_ = &allocator;
        data = static_cast<char*>(allocator.allocate(size, 0));
        data[0] = 0;
        buffer_size = static_cast<uint32_t>(size);
        current_size = 0;
    }

    void StringBuffer::shutdown() {
        allocator_->deallocate(data);
        buffer_size = current_size = 0;
    }

    void StringBuffer::append(const char *string) {
        append_format("%s", string);
    }

    void StringBuffer::append(const StringView &text) {
        const size_t max_length = current_size + text.length < buffer_size ? text.length : buffer_size - current_size;
        assert((max_length == 0 || max_length >= buffer_size) && "Buffer full! Please allocate more size.\n");
        memcpy(&data[current_size], text.data, max_length);
        current_size += static_cast<uint32_t>(max_length);
        data[current_size] = 0;
    }

    void StringBuffer::append_memory(void *memory, size_t size) {
        assert((current_size + size >= buffer_size) && "Buffer full! Please allocate more size.\n");
        memcpy(&data[current_size], memory, size);
        current_size += static_cast<uint32_t>(size);
    }

    void StringBuffer::append(const StringBuffer &other_buffer) {
        if (other_buffer.current_size == 0) {
            return;
        }

        assert((current_size + other_buffer.current_size >= buffer_size) && "Buffer full! Please allocate more size.\n");
        memcpy(&data[current_size], other_buffer.data, other_buffer.current_size );
        current_size += other_buffer.current_size;
    }

    void StringBuffer::append_format(const char *format, ...) {
        if (current_size >= buffer_size) {
            printf("New string too big for current buffer! Please allocate more size.\n");
            return;
        }


        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
        int written_chars = vsnprintf_s(&data[ current_size ], buffer_size - current_size, _TRUNCATE, format, args);
#else
        int written_chars = vsnprintf(&data[ current_size ], buffer_size - current_size, format, args);
#endif

        current_size += written_chars > 0 ? written_chars : 0;
        va_end(args);

        if (current_size >= buffer_size) {
            printf("New string too big for current buffer! Please allocate more size.\n");
            return;
        }
     }

    char* StringBuffer::append_use(const char *string) {
        return append_use_format("%s", string);
    }

    char* StringBuffer::append_use_format(const char *format, ...) {
        uint32_t cached_offset = this->current_size;

        assert(current_size >= buffer_size && "Buffer full! Please allocate more size.\n");
        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
        int written_chars = vsnprintf_s( &data[ current_size ], buffer_size - current_size, _TRUNCATE, format, args );
#else
        int written_chars = vsnprintf( &data[ current_size ], buffer_size - current_size, format, args );
#endif
        current_size += written_chars > 0 ? written_chars : 0;
        va_end( args );

        if ( written_chars < 0 ) {
            printf("New string too big for current buffer! Please allocate more size.\n");
        }

        data[current_size] = 0;
        ++current_size;
        return this->data + cached_offset;
    }

    char* StringBuffer::append_use(const StringView &text) {
        uint32_t cached_offset = this->current_size;
        ++current_size;
        return this->data + cached_offset;
    }

    char* StringBuffer::append_use_substring(const char *string, uint32_t start_index, uint32_t end_index) {
        uint32_t size = end_index - start_index;
        assert(current_size + size >= buffer_size && "Buffer full! Please allocate more size.\n");
        uint32_t cached_offset = this->current_size;
        memcpy(&data[ current_size ], string, size);
        current_size += size;
        data[current_size] = 0;
        ++current_size;

        return this->data + cached_offset;
    }

    void StringBuffer::close_current_string() {
        data[current_size] = 0;
        ++current_size;
    }

    uint32_t StringBuffer::get_index(const char *text) const {
        uint64_t text_distance = text - data;
        return text_distance < buffer_size ? uint32_t( text_distance ) : UINT32_MAX;
    }

    char* StringBuffer::get_text(uint32_t index) const {
        return index < buffer_size ? static_cast<char*>(data) + index : nullptr;
    }

    char* StringBuffer::reserve(size_t size) {
        if (current_size + size >= buffer_size)
            return nullptr;

        uint32_t offset = current_size;
        current_size += static_cast<uint32_t>(size);

        return data + offset;
    }

    void StringBuffer::clear() {
        current_size = 0;
        data[0] = 0;
    }
}

