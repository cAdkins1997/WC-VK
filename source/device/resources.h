
#pragma once
#include "../vkcommon.h"

struct FrameData {
    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;

    vk::Semaphore swapchainSemaphore;
    vk::Semaphore renderSemaphore;
    vk::Fence renderFence;
};

struct Buffer {
    vk::Buffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct Image {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    vk::Extent3D imageExtent;
    VkFormat imageFormat;
    VkSampler sampler;
    size_t handle;
};

struct Vertex {
    glm::vec3 position;
    float uv_x;
    glm::vec3 normal;
    float uv_y;
    glm::vec4 color;
};

struct Surface {
    uint32_t startIndex{};
    uint32_t count{};
};

struct Mesh {
    std::string name{};
    std::vector<Surface> surfaces;
};

enum class TextureHandle : uint32_t { Invalid = 0 };
enum class BufferHandle : uint32_t { Invalid = 0 };

struct MeshBuffer {
    BufferHandle indexBuffer;
    BufferHandle vertexBuffer;
    VkDeviceAddress vertexBufferAddress;
};

struct PushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBuffer;
};

struct Shader {
    VkShaderModule module;
};


struct Pipeline {
    vk::Pipeline pipeline;
};
