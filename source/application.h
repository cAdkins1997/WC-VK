
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device/device.hpp"
#include "context.h"

#include "pipelines/descriptors.h"

#include "frustum.h"

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
        VkImage drawHandle{};
        vk::Extent2D drawImageExtent;

        Image depthImage;
        VkImage depthHandle{};

        Buffer sceneData;

        vk::RenderingAttachmentInfo drawAttachment;
        vk::RenderingAttachmentInfo depthAttachment;

    private:
        core::Device device;

        std::vector<std::shared_ptr<Mesh>> testMeshes;
    };
}

#endif //APPLICATION_H
