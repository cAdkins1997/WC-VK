
#include "shaders.h"

bool shaders::load_shader_module(VkDevice device, const char *filePath, VkShaderModule* outputModule) {

    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    size_t fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    if (!file.is_open()) {
        return false;
    }

    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCI{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    shaderModuleCI.pNext = nullptr;
    shaderModuleCI.codeSize = buffer.size() * sizeof(uint32_t);
    shaderModuleCI.pCode = buffer.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }

    *outputModule = shaderModule;
    return true;
}
