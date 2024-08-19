#ifndef CONTEXT_H
#define CONTEXT_H

#include "device.h"

struct Pipeline;
struct Buffer;
struct Image;

struct GraphicsContext {
    void begin();
    void end();
    void resource_barrier();
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