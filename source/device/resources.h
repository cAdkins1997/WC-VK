
#pragma once
#include <EASTL/compare.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

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
    eastl::string name{};
    eastl::vector<Surface> surfaces;
};

enum class TextureHandle : uint32_t { Invalid = 0 };
enum class BufferHandle : uint32_t { Invalid = 0 };

struct MeshBuffer {
    BufferHandle indexBuffer;
    BufferHandle vertexBuffer;
    vk::DeviceAddress deviceAddress;
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

struct Descriptor {
    VkBuffer bufer;
    vk::DeviceAddress deviceAddress;
    vk::DescriptorSetLayout layout;
    size_t offset;
    size_t size;
};