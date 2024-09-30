
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
        void bind_pipeline(const Pipeline& pipeline);
        void bind_index_buffer(vk::Buffer indexBuffer) const;
        void bind_vertex_buffer(vk::Buffer vertexBuffer) const;

        template<typename T>
        void set_push_constants(const vk::ShaderStageFlags shaderStage, T* pushConstants) {
            _commandBuffer.pushConstants(_pipeline.pipelineLayout, shaderStage, 0, sizeof(T), &pushConstants);
        }

        void draw();

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
        explicit UploadContext(const vk::CommandBuffer& commandBuffer, const VmaAllocator& allocator);

        void begin();
        void end();

        template<typename T>
        void upload_buffer(std::vector<T> src, Buffer& dst, const size_t size) {
            Buffer stagingBuffer = make_staging_buffer(size);

            memcpy(stagingBuffer.info.pMappedData, src.data(), size);

            vk::BufferCopy copy(0, 0, size);
            _commandBuffer.copyBuffer(stagingBuffer.buffer, dst.buffer, 1, &copy);
        }

        MeshBuffer upload_mesh(Buffer &vertexBuffer, Buffer &indexBuffer, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);

        void upload_texture();

    private:
        Buffer make_staging_buffer(size_t allocSize);

    public:
        VmaAllocator _allocator{};
        vk::CommandBuffer _commandBuffer;
        Pipeline _pipeline;
    };
}
