
#pragma once
#include "vkinit.h"
#include <iostream>

#include "device/resources.h"

namespace wcvk {
    struct GraphicsContext {
        explicit GraphicsContext(const vk::CommandBuffer& _commandBuffer);

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

        vk::CommandBuffer commandBuffer{};
    };

    struct ComputeContext {
        explicit ComputeContext(const vk::CommandBuffer& commandBuffer);

        void begin();
        void end();
        void image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
        void copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
        void resource_barrier();
        void set_pipeline(const Pipeline& pipeline);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        vk::CommandBuffer _commandBuffer;
        Pipeline _pipeline;
    };

    struct RaytracingContext {

    };
}
