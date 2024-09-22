
#include "context.h"

namespace wcvk::commands {
    GraphicsContext::GraphicsContext(const vk::CommandBuffer& commandBuffer) : _commandBuffer(commandBuffer) {}

    void GraphicsContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        _commandBuffer.begin(&beginInfo) == vk::Result::eSuccess && "Failed to end graphics command buffer\n";
    }

    void GraphicsContext::end() {
        _commandBuffer.end();
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
        vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
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
        vkCmdPipelineBarrier2(_commandBuffer, &dependencyInfo);
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
        _commandBuffer.pipelineBarrier2(&dependencyInfo);
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

        _commandBuffer.blitImage2(&blitInfo);
    }

    void GraphicsContext::set_up_render_pass(vk::Extent2D extent, const vk::RenderingAttachmentInfo *drawImage, const vk::RenderingAttachmentInfo *depthImage) const {
        vk::Rect2D renderArea;
        renderArea.extent = extent;
        vk::RenderingInfo renderInfo;
        renderInfo.renderArea = renderArea;
        renderInfo.pColorAttachments = drawImage;
        renderInfo.pDepthAttachment = depthImage;
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        _commandBuffer.beginRendering(&renderInfo);
    }

    void GraphicsContext::set_viewport(float x, float y, float minDepth, float maxDepth) {
        vk::Viewport viewport(0, 0, x, y);
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;
        _commandBuffer.setViewport(0, 1, &viewport);
    }

    void GraphicsContext::set_viewport(vk::Extent2D extent, float minDepth, float maxDepth) {
        vk::Viewport viewport(0, 0, extent.width, extent.height);
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;
        _commandBuffer.setViewport(0, 1, &viewport);
    }

    void GraphicsContext::set_scissor(uint32_t x, uint32_t y) {
        vk::Rect2D scissor;
        vk::Extent2D extent {x, y};
        scissor.extent = extent;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        _commandBuffer.setScissor(0, 1, &scissor);
    }

    void GraphicsContext::set_scissor(vk::Extent2D extent) {
        vk::Rect2D scissor;
        scissor.extent = extent;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        _commandBuffer.setScissor(0, 1, &scissor);
    }

    void GraphicsContext::bind_pipeline(const Pipeline &pipeline) {
        _pipeline = pipeline;
        _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, 1, &pipeline.set, 0, nullptr);
    }

    void GraphicsContext::draw() {
        _commandBuffer.draw(3, 1, 0, 0);
        _commandBuffer.endRendering();
    }

    ComputeContext::ComputeContext(const vk::CommandBuffer &commandBuffer) : _commandBuffer(commandBuffer) {}

    void ComputeContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        _commandBuffer.begin(&beginInfo) == vk::Result::eSuccess && "Failed to end compute command buffer\n";
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

    void ComputeContext::bind_pipeline(const Pipeline &pipeline) {
        _pipeline = pipeline;
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.pipelineLayout, 0, 1, &pipeline.set, 0, nullptr);
        _commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline.pipeline);
    }

    void ComputeContext::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        _commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
    }

    UploadContext::UploadContext(const vk::CommandBuffer &commandBuffer) : _commandBuffer(commandBuffer) {}

    void UploadContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        _commandBuffer.begin(&beginInfo) == vk::Result::eSuccess && "Failed to begin upload buffer\n";
    }

    void UploadContext::end() {
        _commandBuffer.end();
    }
}
