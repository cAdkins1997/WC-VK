
#include "sceneresources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

void SceneDesc::init(const wcvk::scene::glTF& _scene, wcvk::core::Device& _device, wcvk::allocators::Allocator* _allocator) {
    scene = _scene;
    device = &_device;
    allocator = _allocator;
    images.init(allocator, scene.imageCount * sizeof(Image));
    samplers.init(allocator, scene.samplerCount * sizeof(vk::Sampler));
    buffers.init(allocator, scene.bufferCount * sizeof(Buffer));1

}

void SceneDesc::build() {
    build_images();
    build_samplers();
    build_buffers();
}

eastl::optional<Image> SceneDesc::process_image_uri(const wcvk::scene::Image& image, wcvk::allocators::Allocator* allocator) const {
    Image result{};
    const char* uri = image.uri.data;
    std::ifstream file(uri, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::printf( "Error: uri file %s does not exists.\n", uri);
        return{};
    }

    int32_t comp, width, height;
    if (uint8_t* imageData = stbi_load(uri, &width, &height, &comp, 4); !imageData) {
        printf("Error loading texture %s", uri);
        return{};
    }

    vk::Extent3D extent {static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(comp)};
    result = device->create_image(extent, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, true);

    file.close();

    return result;
}

void SceneDesc::build_images() {
    for (uint32_t i = 0; i < scene.imageCount; ++i) {
        wcvk::scene::Image& gltfImage = scene.images[i];
        if (auto wcvkImage = process_image_uri(gltfImage, allocator); wcvkImage.has_value()) {
            images.push(wcvkImage.value());
        }
    }
}

void SceneDesc::build_samplers() {
    for (uint32_t i = 0; i < scene.samplerCount; ++i) {
        wcvk::scene::Sampler& gltfSampler = scene.samplers[i];
        vk::Sampler sampler = device->create_sampler(
            static_cast<vk::Filter>(gltfSampler.minFilter),
            static_cast<vk::Filter>(gltfSampler.magFilter),
            vk::SamplerMipmapMode::eNearest);
        samplers.push(sampler);
    }
}

void SceneDesc::build_buffers() {
    for (uint32_t i = 0; i < scene.bufferCount; ++i) {
        wcvk::scene::Buffer& gltfBuffer = scene.buffers[i];
        Buffer buffer = device->create_buffer(gltfBuffer.byteLength, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        buffers.push(buffer);
    }
}
