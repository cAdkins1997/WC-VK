#include "meshes.h"

namespace wcvk::meshes {
    /*MeshBuffer upload_mesh(core::Device device, commands::UploadContext& context, eastl::span<uint32_t> indices, eastl::span<Vertex> vertices) {
        const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
        const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

        MeshBuffer newSurface;

        newSurface.vertexBuffer = device.create_buffer(
            vertexBufferSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            VMA_MEMORY_USAGE_GPU_ONLY
            );

        vk::BufferDeviceAddressInfo deviceAddressInfo;
        newSurface.deviceAddress = device.device.getBufferAddress(&deviceAddressInfo);

        newSurface.indexBuffer = device.create_buffer(indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY);

        context.begin();
        context.upload_buffer(device, vertices, newSurface.vertexBuffer, vertexBufferSize);
        context.upload_buffer(device, indices, newSurface.indexBuffer, indexBufferSize);
        context.end();

        //device.submit_upload_work(context);

        return newSurface;
    }*/
}
