
#pragma once
#include "vkinit.h"
#include <iostream>

#include "device/device.hpp"

namespace wcvk::core {
    class Device;
}

namespace wcvk::commands {
    struct GraphicsContext {
        explicit GraphicsContext(const vk::CommandBuffer& commandBuffer);

        void begin();
        void end();
        void memory_barrier(
            VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask,
            VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask);
        void buffer_barrier(
            VkBuffer buffer, VkDeviceSize offset,
            VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask,
            VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask);
        void image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
        void copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
        void set_up_render_pass(vk::Extent2D extent, const vk::RenderingAttachmentInfo *drawImage, const vk::RenderingAttachmentInfo *depthImage) const;
        void set_viewport(float x, float y, float minDepth, float maxDepth);
        void set_viewport(vk::Extent2D extent, float minDepth, float maxDepth);
        void set_scissor(uint32_t x, uint32_t y);
        void set_scissor(vk::Extent2D extent);
        void set_push_constants(vk::ShaderStageFlags shaderStages, uint32_t offset, PushConstants& pushConstants);
        void bind_pipeline(const Pipeline& pipeline);
        void bind_index_buffer(vk::Buffer indexBuffer) const;
        void bind_vertex_buffer(vk::Buffer vertexBuffer) const;

        template<typename T>
        void set_push_constants(const vk::ShaderStageFlags shaderStage, T pushConstants) {
            _commandBuffer.pushConstants(_pipeline.pipelineLayout, shaderStage, 0, sizeof(T), &pushConstants);
        }

        void draw(uint32_t count, uint32_t startIndex);

        vk::CommandBuffer _commandBuffer;
        Pipeline _pipeline;
    };

    struct ComputeContext {
        explicit ComputeContext(const vk::CommandBuffer& commandBuffer);

        void begin();
        void end();
        void image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
        void copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
        void resource_barrier();
        void bind_pipeline(const Pipeline& pipeline);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        vk::CommandBuffer _commandBuffer;
        Pipeline _pipeline;
    };

    struct RaytracingContext {

    };

    struct UploadContext {
        explicit UploadContext(const vk::Device& device, const vk::CommandBuffer& commandBuffer, const VmaAllocator& allocator);

        void begin();
        void end();

        void image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
        void copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);

        MeshBuffer upload_mesh(std::span<Vertex> vertices, std::span<uint32_t> indices);

        void upload_texture();
        void upload_image(void* data, Image& image);

    private:
        Buffer make_staging_buffer(size_t allocSize);
        Buffer create_device_buffer(size_t size, VkBufferUsageFlags bufferUsage);

    public:
        VmaAllocator _allocator{};
        vk::CommandBuffer _commandBuffer;
        vk::Device _device;
        Pipeline _pipeline;
    };
}
