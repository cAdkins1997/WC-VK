#pragma once
#include "device/device.hpp"

#include <glm/gtx/quaternion.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/base64.hpp>
#include <fastgltf/util.hpp>

namespace wcvk::GLTF {
    struct Material {
        MaterialPipeline opaquePipeline;
        MaterialPipeline transparentPipeline;
        vk::DescriptorSetLayout materialLayout;

        struct MatConstants {
            glm::vec4 baseColorFactors{};
            glm::vec4 mrFactors{};
            glm::vec4 extra[14];
        };

        struct MaterialResources {
            Image colorImage;
            vk::Sampler colorSampler;
            Image metalRoughImage;
            vk::Sampler metalRoughSampler;
            vk::Buffer dataBuffer;
            uint32_t dataBufferOffset;
        };

        DescriptorWriter writer;

        void build_pipelines(core::Device* device, vk::DescriptorSetLayout gpuSceneDataDescriptorLayout);
        void clear_resources(vk::Device device);
        MaterialInstance write_material(vk::Device device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocator& descriptorAllocator);
    };

    struct DrawContext;

    class IRenderable {

        virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
    };

    struct Node : IRenderable {

        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;

        glm::mat4 localTransform;
        glm::mat4 worldTransform;

        void refreshTransform(const glm::mat4& parentMatrix);

        void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
    };

    struct MeshNode : Node {

        std::shared_ptr<Mesh> mesh;

        void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
    };

    struct RenderObject {
        uint32_t indexCount;
        uint32_t firstIndex;
        vk::Buffer indexBuffer;

        MaterialInstance* material;

        glm::mat4 transform;
        vk::DeviceAddress vertexBufferAddress;
    };

    struct DrawContext {
        std::vector<RenderObject> OpaqueSurfaces;
        std::vector<RenderObject> TransparentSurfaces;
    };

    struct SceneDesc {
        DrawContext mainDrawContext;
        std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;
    };

    struct LoadedGLTF : IRenderable {

        std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
        std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
        std::unordered_map<std::string, Image> images;
        std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

        std::vector<std::shared_ptr<Node>> topNodes;

        std::vector<vk::Sampler> samplers;

        DescriptorAllocator descriptorPool;

        Buffer materialDataBuffer;

        core::Device* creator;

        Image checkerboardImage;
        Image whiteImage;

        ~LoadedGLTF() { clearAll(); };

        void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

    private:

        void clearAll();
    };

    std::optional<std::shared_ptr<LoadedGLTF>> load_GLTF(
        core::Device& device,
        commands::UploadContext& uploadContext,
        std::string_view filepath,
        Material& metalRoughMaterial
        );
    vk::Filter extract_filter(fastgltf::Filter filter);
    vk::SamplerMipmapMode extract_mipmap_mode(fastgltf::Filter filter);
    std::optional<Image> load_image(
        core::Device& device,
        commands::UploadContext& uploadContext,
        fastgltf::Asset& asset,
        fastgltf::Image& image);
}

