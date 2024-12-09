
#include "context.h"

namespace wcvk::commands {
    GraphicsContext::GraphicsContext(const vk::CommandBuffer& commandBuffer) : _commandBuffer(commandBuffer) {}

    void GraphicsContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        vk_check(_commandBuffer.begin(&beginInfo), "Failed to begine command buffer");
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

        vk::ImageAspectFlags aspectMask = (newLayout == (vk::ImageLayout)VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? (vk::ImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT : (vk::ImageAspectFlags)VK_IMAGE_ASPECT_COLOR_BIT;
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

    void GraphicsContext::copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {
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

    void GraphicsContext::set_push_constants(vk::ShaderStageFlags shaderStages, uint32_t offset, PushConstants &pushConstants) {
        _commandBuffer.pushConstants(_pipeline.pipelineLayout, shaderStages, offset, sizeof(PushConstants), &pushConstants);
    }

    void GraphicsContext::bind_pipeline(const Pipeline &pipeline) {
        _pipeline = pipeline;
        _commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
        _commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, 1, &pipeline.set, 0, nullptr);
    }

    void GraphicsContext::bind_index_buffer(vk::Buffer indexBuffer) const {
        _commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
    }

    void GraphicsContext::bind_vertex_buffer(vk::Buffer vertexBuffer) const {
        vk::DeviceSize offsets[] = {0};
        _commandBuffer.bindVertexBuffers(0, vertexBuffer, offsets);
    }

    void GraphicsContext::draw(uint32_t count, uint32_t startIndex) {
        _commandBuffer.drawIndexed(count, 1, startIndex, 0, 0);
        _commandBuffer.endRendering();
    }

    ComputeContext::ComputeContext(const vk::CommandBuffer &commandBuffer) : _commandBuffer(commandBuffer) {}

    void ComputeContext::begin() {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        vk_check(_commandBuffer.begin(&beginInfo), "Failed to begin command buffer");
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

    UploadContext::UploadContext(const vk::Device& device, const vk::CommandBuffer &commandBuffer, const VmaAllocator& allocator) :
    _device(device) , _allocator(allocator), _commandBuffer(commandBuffer) {}

    void UploadContext::begin() const {
        _commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        vk_check(_commandBuffer.begin(&beginInfo), "Failed to begin command buffer");
    }

    void UploadContext::end() const {
        _commandBuffer.end();
    }

    void UploadContext::image_barrier(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout) const {

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

    void UploadContext::buffer_barrier(
        vk::Buffer buffer,
        vk::DeviceSize offset,
        vk::PipelineStageFlags srcStageFlags,
        vk::AccessFlags srcAccessMask,
        vk::PipelineStageFlags dstStageFlags,
        vk::AccessFlags dstAccessMask) const
    {
        vk::BufferMemoryBarrier barrier(srcAccessMask, dstAccessMask);
        barrier.buffer = buffer;
        barrier.offset = offset;
        barrier.size = vk::WholeSize;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

        _commandBuffer.pipelineBarrier(
            srcStageFlags,
            dstStageFlags,
            {},
            0,
            nullptr,
            1,
            &barrier,
            0,
            nullptr
            );
    }

    void UploadContext::copy_buffer(
        vk::Buffer bufferSrc,
        vk::Buffer bufferDst,
        vk::DeviceSize srcOffset,
        vk::DeviceSize dstOffset,
        vk::DeviceSize dataSize)
    {
        vk::BufferCopy bufferCopy {srcOffset, dstOffset, dataSize};
        _commandBuffer.copyBuffer(bufferSrc, bufferDst, 1, &bufferCopy);
    }

    void UploadContext::copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize) {
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

    MeshBuffer UploadContext::upload_mesh(const eastl::vector<Vertex> &vertices, const eastl::vector<uint32_t> &indices) {
        const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
        const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

        MeshBuffer newMesh{};
        newMesh.vertexBuffer = create_device_buffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
        newMesh.indexBuffer = create_device_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        Buffer stagingBuffer = make_staging_buffer(vertexBufferSize + indexBufferSize);

        void* data = stagingBuffer.info.pMappedData;
        vmaMapMemory(_allocator, stagingBuffer.allocation, &data);
        memcpy(data, vertices.data(), vertexBufferSize);
        memcpy(static_cast<char*>(data) + vertexBufferSize, indices.data(), indexBufferSize);
        vmaUnmapMemory(_allocator, stagingBuffer.allocation);

        vk::BufferDeviceAddressInfo deviceAddressInfo;
        deviceAddressInfo.buffer = newMesh.vertexBuffer.buffer;
        newMesh.deviceAddress = _device.getBufferAddress(&deviceAddressInfo);

        vk::BufferCopy vertexCopy(0, 0, vertexBufferSize);
        vk::BufferCopy indexCopy(vertexBufferSize, 0, indexBufferSize);

        _commandBuffer.copyBuffer(stagingBuffer.buffer, newMesh.vertexBuffer.buffer, 1, &vertexCopy);
        _commandBuffer.copyBuffer(stagingBuffer.buffer, newMesh.indexBuffer.buffer, 1, &indexCopy);

        return newMesh;
    }

    void UploadContext::upload_image(void* data, Image &image) {
        vk::Extent3D extent = image.get_extent();
        size_t imageDataSize = extent.depth * extent.width * extent.height * 4;

        Buffer stagingBuffer = make_staging_buffer(imageDataSize);

        memcpy(stagingBuffer.info.pMappedData, data, imageDataSize);

        image_barrier(image.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        vk::BufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = extent;

        _commandBuffer.copyBufferToImage(stagingBuffer.buffer, image.image, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

        image_barrier(image.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    void UploadContext::upload_uniform(void* data, size_t dataSize, Buffer &uniform) {
        if (uniform.memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            memcpy(uniform.get_mapped_data(), data, dataSize);
            vk_check(
                vmaFlushAllocation(_allocator, uniform.allocation, 0, VK_WHOLE_SIZE),
                "Failed to flush buffer"
                );

            buffer_barrier(
                uniform.buffer,
                0,
                vk::PipelineStageFlagBits::eHost,
                vk::AccessFlagBits::eHostWrite,
                vk::PipelineStageFlagBits::eTransfer,
                vk::AccessFlagBits::eTransferRead
            );
        }
        else {
            Buffer stagingBuffer = make_staging_buffer(dataSize);
            memcpy(stagingBuffer.get_mapped_data(), data, dataSize);
            vk_check(
            vmaFlushAllocation(_allocator, uniform.allocation, 0, VK_WHOLE_SIZE),
            "Failed to flush buffer"
            );

            buffer_barrier(
                stagingBuffer.buffer,
                0,
                vk::PipelineStageFlagBits::eHost,
                vk::AccessFlagBits::eHostWrite,
                vk::PipelineStageFlagBits::eTransfer,
                vk::AccessFlagBits::eTransferRead
            );

            copy_buffer(stagingBuffer.buffer, uniform.buffer, 0, 0, dataSize);

            buffer_barrier(
                uniform.buffer,
                0,
                vk::PipelineStageFlagBits::eTransfer,
                vk::AccessFlagBits::eTransferWrite,
                vk::PipelineStageFlagBits::eVertexShader,
                vk::AccessFlagBits::eUniformRead
            );
        }
    }

    Buffer UploadContext::make_staging_buffer(size_t allocSize) {
        VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        Buffer newBuffer{};

        vk_check(
            vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info),
                "Failed to create staging buffer"
            );

        return newBuffer;
    }

    Buffer UploadContext::create_device_buffer(size_t size, VkBufferUsageFlags bufferUsage) {
        VmaAllocationCreateInfo vmaAI{};
        vmaAI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaAI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBufferCreateInfo deviceBufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        deviceBufferInfo.pNext = nullptr;
        deviceBufferInfo.size = size;
        deviceBufferInfo.usage = bufferUsage;

        Buffer deviceBuffer{};
        vk_check(
            vmaCreateBuffer(_allocator, &deviceBufferInfo, &vmaAI, &deviceBuffer.buffer, &deviceBuffer.allocation, &deviceBuffer.info),
                "Failed to create upload context device buffer"
            );

        return deviceBuffer;
    }
}
