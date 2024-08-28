
#ifndef RESOURCES_H
#define RESOURCES_H

#include "../vkcommon.h"

struct FrameData {
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore swapchainSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;
};

struct Buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct Image {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
    VkSampler sampler;
    size_t handle;
};

struct Shader {
    VkShaderModule module;
};


struct Pipeline {
    VkPipeline pipeline;
};

#endif //RESOURCES_H
