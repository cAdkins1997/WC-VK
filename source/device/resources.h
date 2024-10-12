
#pragma once

#include "../glmdefines.h"

#include <vector>
#include <string>
#include <functional>

#include "../frustum.h"
#include "../vkcommon.h"
#include "../pipelines/descriptors.h"

enum class DeviceAddress : uint64_t { Invalid = 0 };

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)();
        }

        deletors.clear();
    }
};

struct FrameData {
    DeletionQueue deletionQueue;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
    vk::Semaphore swapchainSemaphore;
    vk::Semaphore renderSemaphore;
    vk::Fence renderFence;

    DescriptorAllocator frameDescriptors;
};

struct Buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VkMemoryPropertyFlags memoryProperties;

    [[nodiscard]] void* get_mapped_data() const { return info.pMappedData; }
};

struct Image {
    VkImage image;
    vk::ImageView imageView;
    VmaAllocation allocation;
    vk::Extent3D imageExtent;
    vk::Format imageFormat;
    vk::Sampler sampler;
    size_t handle;

    [[nodiscard]] VkImage get_handle() const { return image; }
    [[nodiscard]] vk::ImageView get_view() const { return imageView; }
    [[nodiscard]] VmaAllocation get_allocation() const { return allocation; }
    [[nodiscard]] vk::Extent3D get_extent() const { return imageExtent; }
    [[nodiscard]] uint32_t get_width() const { return imageExtent.width; }
    [[nodiscard]] uint32_t get_height() const { return imageExtent.height; }
    [[nodiscard]] vk::Format get_format() const { return imageFormat; }
    [[nodiscard]] vk::Sampler get_sampler() const { return sampler; }
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
    std::string name{};
    std::vector<Surface> surfaces;
    MeshBuffer mesh;
};

struct PushConstants {
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

struct SceneData {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 perspective;
    wcvk::Frustum frustum;
    glm::vec3 lightPos;
};

struct Plane {
    glm::vec3 normal {0.0f, 1.0f, 0.0f};
    float distance = 0.0f;
};