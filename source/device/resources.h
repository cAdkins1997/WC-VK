
#pragma once
#include <EASTL/compare.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

#include "../vkcommon.h"
#include "../pipelines/descriptors.h"

struct FrameData {
    vk::CommandPool commandPool;
    vk::CommandBuffer graphicsCommandBuffer;
    vk::CommandBuffer computeCommandBuffer;
    vk::CommandBuffer uploadCommandBuffer;

    vk::Semaphore swapchainSemaphore;
    vk::Semaphore renderSemaphore;
    vk::Fence renderFence;

    DescriptorAllocator frameDescriptors;
};

struct Buffer {
    vk::Buffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct Image {
    VkImage image;
    vk::ImageView imageView;
    VmaAllocation allocation;
    vk::Extent3D imageExtent;
    vk::Format imageFormat;
    vk::Sampler sampler;
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

struct MeshBuffer {
    Buffer indexBuffer;
    Buffer vertexBuffer;
    vk::DeviceAddress deviceAddress;
};

struct Mesh {
    eastl::string name{};
    eastl::vector<Surface> surfaces;
    MeshBuffer mesh;
};

struct PushConstants {
    glm::mat4 worldMatrix;
    vk::DeviceAddress vertexBuffer;
};

struct Shader {
    vk::ShaderModule module;
};

struct Pipeline {
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSet set;
    vk::DescriptorSetLayout descriptorLayout;
};

struct ProjectionData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
};