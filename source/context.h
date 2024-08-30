#ifndef CONTEXT_H
#define CONTEXT_H

#include "device/device.h"

struct Pipeline;
struct Buffer;
struct Image;

class Device;

struct GraphicsContext {
    explicit GraphicsContext(VkCommandBuffer _commandBuffer);

    void begin();
    void end();
    void memory_barrier(
        VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask,
        VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask);
    void buffer_barrier(
        VkBuffer buffer, VkDeviceSize offset,
        VkPipelineStageFlags2 srcStageFlags, VkAccessFlags2 srcAccessMask,
        VkPipelineStageFlags2 dstStageFlags, VkAccessFlags2 dstAccessMask);
    void image_barrier(VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
    void copy_image(VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);

    void draw();

    void set_pipeline(Pipeline pipeline);
    void set_vertex_buffer(Buffer vertexBuffer);
    void set_index_buffer(Buffer indexBuffer);

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

struct ComputeContext {
    void begin();
    void end();
    void resource_barrier();
    void dispatch();

    void set_pipeline(Pipeline pipeline);

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};

struct RaytracingContext {

};

struct UploadContext {
    UploadContext(VkCommandBuffer _commandBuffer, Device& _device);

    void begin();
    void end();
    void resource_barrier();

    VkDeviceAddress upload_buffer(Buffer& buffer, size_t size);
    void upload_texture(Image);

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    Device& device;
};

#endif