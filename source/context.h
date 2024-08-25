#ifndef CONTEXT_H
#define CONTEXT_H

#include "device.h"

struct Pipeline;
struct Buffer;
struct Image;

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
    void begin();
    void end();
    void resource_barrier();

    void upload_buffer(Buffer);
    void upload_texture(Image);
};

#endif