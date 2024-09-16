
#include "context.h"

namespace wcvk {
    GraphicsContext::GraphicsContext(const vk::CommandBuffer& _commandBuffer) : commandBuffer(_commandBuffer) {}

    void GraphicsContext::begin() {
        commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        assert(commandBuffer.begin(&beginInfo) == vk::Result::eSuccess && "Failed to end graphics command buffer\n");
    }

    void GraphicsContext::end() {
        commandBuffer.end();
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

    void GraphicsContext::image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout) {

        vk::ImageMemoryBarrier2 imageBarrier(
            vk::PipelineStageFlagBits2::eAllCommands,vk::AccessFlagBits2::eMemoryWrite,
            vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead
            );


        imageBarrier.oldLayout = currentLayout;
        imageBarrier.newLayout = newLayout;

        vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
        imageBarrier.subresourceRange = vk::ImageSubresourceRange(
            aspectMask,
            0,
            vk::RemainingMipLevels,
            0,
            vk::RemainingArrayLayers
            );
        imageBarrier.image = image;

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &imageBarrier;
        commandBuffer.pipelineBarrier2(&dependencyInfo);
    }

    void GraphicsContext::copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {\
        vk::ImageBlit2 blitRegion;

        blitRegion.srcOffsets[1].x = srcSize.width;
        blitRegion.srcOffsets[1].y = srcSize.height;
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = dstSize.width;
        blitRegion.dstOffsets[1].y = dstSize.height;
        blitRegion.dstOffsets[1].z = 1;

        blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;

        blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstSubresource.mipLevel = 0;

        vk::BlitImageInfo2 blitInfo(
            src,
            vk::ImageLayout::eTransferSrcOptimal,
            dst,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &blitRegion,
            vk::Filter::eLinear
            );

        commandBuffer.blitImage2(&blitInfo);
    }

    ComputeContext::ComputeContext(const vk::CommandBuffer &commandBuffer) : _commandBuffer(commandBuffer) {}

    void ComputeContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        assert(_commandBuffer.begin(&beginInfo) == vk::Result::eSuccess && "Failed to end compute command buffer\n");
    }

    void ComputeContext::end() {
        _commandBuffer.end();
    }

    void ComputeContext::image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout) {
        vk::ImageMemoryBarrier2 imageBarrier(
    vk::PipelineStageFlagBits2::eAllCommands,vk::AccessFlagBits2::eMemoryWrite,
    vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead
    );


        imageBarrier.oldLayout = currentLayout;
        imageBarrier.newLayout = newLayout;

        vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
        imageBarrier.subresourceRange = vk::ImageSubresourceRange(
            aspectMask,
            0,
            vk::RemainingMipLevels,
            0,
            vk::RemainingArrayLayers
            );
        imageBarrier.image = image;

        vk::DependencyInfo dependencyInfo;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &imageBarrier;
        _commandBuffer.pipelineBarrier2(&dependencyInfo);
    }

    void ComputeContext::copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {
        vk::ImageBlit2 blitRegion;

        blitRegion.srcOffsets[1].x = srcSize.width;
        blitRegion.srcOffsets[1].y = srcSize.height;
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = dstSize.width;
        blitRegion.dstOffsets[1].y = dstSize.height;
        blitRegion.dstOffsets[1].z = 1;

        blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;

        blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstSubresource.mipLevel = 0;

        vk::BlitImageInfo2 blitInfo(
            src,
            vk::ImageLayout::eTransferSrcOptimal,
            dst,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &blitRegion,
            vk::Filter::eLinear
            );

        _commandBuffer.blitImage2(&blitInfo);
    }

    void ComputeContext::set_pipeline(const Pipeline &pipeline) {
        _pipeline = pipeline;
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.pipelineLayout, 0, 1, &pipeline.set, 0, nullptr);
        _commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline.pipeline);
    }

    void ComputeContext::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        _commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
    };
}
