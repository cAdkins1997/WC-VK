
#pragma once
#include "vkinit.h"
#include <iostream>

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
        void begin();
        void end();
        void resource_barrier();
        void dispatch();

        vk::CommandBuffer commandBuffer;
    };

    struct RaytracingContext {

    };
}