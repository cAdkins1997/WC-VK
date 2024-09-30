
#include "vkcommon.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

/*void* operator new [](size_t size, const char *name, int flags, unsigned debugFlags, const char *file, int line) {
    return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
    return new uint8_t[size];
}*/
void vk_check(const vk::Result result, const std::string& outputString) {
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error((outputString + ' ' + vk::to_string(result) + '\n'));
    }
}
