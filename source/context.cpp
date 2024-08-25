
#include "context.h"

GraphicsContext::GraphicsContext(VkCommandBuffer _commandBuffer) : commandBuffer(_commandBuffer) {}

void GraphicsContext::begin() {
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_BI(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void GraphicsContext::end() {
    vkEndCommandBuffer(commandBuffer);
}

void GraphicsContext::memory_barrier(
    VkPipelineStageFlags2 srcStageFlags,
    VkAccessFlags2 srcAccessMask,
    VkPipelineStageFlags2 dstStageFlags,
    VkAccessFlags2 dstAccessMask)
{
    VkMemoryBarrier2 memoryBarrier { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, .pNext = nullptr };
    memoryBarrier.srcStageMask = srcStageFlags;
    memoryBarrier.srcAccessMask = srcAccessMask;
    memoryBarrier.dstStageMask = dstStageFlags;
    memoryBarrier.dstAccessMask = dstAccessMask;

    VkDependencyInfo dependencyInfo { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
    dependencyInfo.memoryBarrierCount = 1;
    dependencyInfo.pMemoryBarriers = &memoryBarrier;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

void GraphicsContext::buffer_barrier(
    VkBuffer buffer, VkDeviceSize offset,
    VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask,
    VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask)
{
    VkBufferMemoryBarrier2 bufferBarrier { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, .pNext = nullptr };
    bufferBarrier.srcStageMask = srcStageFlags;
    bufferBarrier.srcAccessMask = srcAccessMask;
    bufferBarrier.dstStageMask = dstStageFlags;
    bufferBarrier.dstAccessMask = dstAccessMask;

    bufferBarrier.buffer = buffer;
    bufferBarrier.offset = offset;

    VkDependencyInfo dependencyInfo { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
    dependencyInfo.bufferMemoryBarrierCount = 1;
    dependencyInfo.pBufferMemoryBarriers = &bufferBarrier;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

void GraphicsContext::image_barrier(VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier2 imageBarrier { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr};

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = vkinit::subresource_range(
        aspectMask,
        0, VK_REMAINING_MIP_LEVELS,
        0, VK_REMAINING_ARRAY_LAYERS);
    imageBarrier.image = image;

    VkDependencyInfo dependencyInfo { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext =nullptr };
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}
