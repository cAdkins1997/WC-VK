
#pragma once
#include "memory.h"
#include "device/device.hpp"
#include "device/resources.h"
#include "pipelines/descriptors.h"

#include <EASTL/shared_ptr.h>

namespace wcvk {
    class Application {
    public:
        Application();
        ~Application();

        void run();
        void draw();

    private:
        void init_descriptors();
        void init_pipeline();

        descriptors::DescriptorAllocator descriptorAllocator;
        Pipeline drawImagePipeline;
        Pipeline trianglePipeline;

        Image drawImage;
        VkImage drawHandle{};
        vk::Extent2D drawImageExtent;

        Image depthImage;
        VkImage depthHandle{};

        vk::RenderingAttachmentInfo drawAttachment;
        vk::RenderingAttachmentInfo depthAttachment;

    private:
        Buffer sceneDataBuffer{};

        vk::DescriptorSetLayout gpuSceneDataDescriptorLayout;
        MaterialInstance defaultData;

    private:
        core::Device device;

        eastl::vector<eastl::shared_ptr<Mesh>> testMeshes;
    };
}
