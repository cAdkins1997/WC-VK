#pragma once
#include "scene.h"
#include <EASTL/optional.h>
#include <fstream>

namespace wcvk {

}
class SceneDesc {
public:
    void init(const wcvk::scene::glTF& _scene, wcvk::core::Device& _device, wcvk::allocators::Allocator* _allocator);
    void build();
    eastl::optional<Image> process_image_uri(const wcvk::scene::Image& image, wcvk::allocators::Allocator* allocator) const;
    eastl::optional<void*> process_buffer_uri(const wcvk::scene::Buffer& buffer, wcvk::allocators::Allocator* allocator) const;

private:
    void build_images();
    void build_samplers();
    void build_buffers();

private:
    wcvk::allocators::Allocator* allocator = nullptr;
    wcvk::scene::glTF scene{};
    wcvk::core::Device* device{};
    wcvk::Array<Image> images{};
    wcvk::Array<vk::Sampler> samplers{};
    wcvk::Array<Buffer> buffers{};
};