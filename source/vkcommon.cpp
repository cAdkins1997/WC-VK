
#include "vkcommon.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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

void vk_check(VkResult result, const std::string &outputString) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(outputString + ' ' + string_VkResult(result) + '\n');
    }
}
