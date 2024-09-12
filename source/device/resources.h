
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
    VkPipeline pipeline;
};

#endif //RESOURCES_H
