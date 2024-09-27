
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device/device.hpp"
#include "context.h"

#include "pipelines/descriptors.h"

namespace wcvk {
    class Application {
    public:
        Application();
        ~Application();

        void run();
        void draw();

    private:
        void init_descriptors();

        DescriptorAllocator descriptorAllocator;
        Pipeline drawImagePipeline;
        Pipeline trianglePipeline;

        Image drawImage;
        VkImage drawHandle;
        vk::Extent2D drawImageExtent;

        eastl::optional<eastl::vector<eastl::shared_ptr<Mesh>>> load_GLTF_meshs(const char* path);

    private:
        core::Device device;

        MeshBuffer meshBuffer{};
        size_t vertSize;
        size_t indexSize;
    };
}

#endif //APPLICATION_H
