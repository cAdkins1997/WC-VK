
#pragma once
#include "scenedesc.h"

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

        DescriptorAllocator descriptorAllocator;
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
        Buffer sceneDataBuffer;

        vk::DescriptorSetLayout gpuSceneDataDescriptorLayout;
        MaterialInstance defaultData;
        GLTF::Material metalRoughMaterial;

        std::shared_ptr<GLTF::LoadedGLTF> sceneDesc;
        GLTF::DrawContext mainDrawContext;
        std::unordered_map<std::string, std::shared_ptr<GLTF::Node>> loadedNodes;

    private:
        core::Device device;

        std::vector<std::shared_ptr<Mesh>> testMeshes;
    };
}
