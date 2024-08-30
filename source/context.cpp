
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

void GraphicsContext::copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {
    VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

    blitRegion.srcOffsets[1].x = srcSize.width;
    blitRegion.srcOffsets[1].y = srcSize.height;
    blitRegion.srcOffsets[1].z = 1;

    blitRegion.dstOffsets[1].x = dstSize.width;
    blitRegion.dstOffsets[1].y = dstSize.height;
    blitRegion.dstOffsets[1].z = 1;

    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;

    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
    blitInfo.dstImage = dst;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.srcImage = src;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.filter = VK_FILTER_LINEAR;
    blitInfo.regionCount = 1;
    blitInfo.pRegions = &blitRegion;

    vkCmdBlitImage2(commandBuffer, &blitInfo);
}

UploadContext::UploadContext(VkCommandBuffer _commandBuffer, Device& _device) :
commandBuffer(_commandBuffer), device(_device) {}

void UploadContext::begin() {
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_BI(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void UploadContext::end() {
    vkEndCommandBuffer(commandBuffer);
}

VkDeviceAddress UploadContext::upload_buffer(Buffer& buffer, size_t size) {
    Buffer stagingBuffer{};

    VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.pNext = nullptr;
    bufferInfo.size = size;

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(device.allocator, &bufferInfo, &vmaallocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, &stagingBuffer.info);

    VkBufferDeviceAddressInfo deviceAddressInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer.buffer
    };

    VkDeviceAddress deviceAddress = vkGetBufferDeviceAddress(device.device, &deviceAddressInfo);

    void* data = stagingBuffer.allocation;

    memcpy(data, buffer.allocation, size);

    VkBufferCopy copy{0};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = size;

    vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, buffer.buffer, 1, &copy);

    vmaDestroyBuffer(device.allocator, stagingBuffer.buffer, stagingBuffer.allocation);

    return deviceAddress;
}

void UploadContext::upload_texture(Image) {
}