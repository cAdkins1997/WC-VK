/*
#pragma once
#include <memory>
#include <filesystem>

#include "glmdefines.h"

#include <glm/gtx/quaternion.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/base64.hpp>
#include <fastgltf/util.hpp>

#include "context.h"
#include "device/resources.h"

#include <ktx.h>
#include <ktxvulkan.h>

namespace wcvk::meshes {

    struct Node {
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;

        glm::mat4 localTransform;
        glm::mat4 worldTransform;

        void refreshTransform(const glm::mat4& parentMatrix);
    };

    struct MeshNode : Node {
        std::shared_ptr<Mesh> mesh;
    };

    struct RenderObject {
        uint32_t indexCount;
        uint32_t firstIndex;
        VkBuffer indexBuffer;

        Material* material;

        glm::mat4 transform;
        VkDeviceAddress vertexBufferAddress;
    };

    struct DrawContext {
        std::vector<RenderObject> opaqueSurfaces;
        std::vector<RenderObject> transparentSurfaces;
    };

    struct GLTF {
        std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
        std::unordered_map<std::string, std::shared_ptr<Image>> images;
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;

        std::vector<std::shared_ptr<Node>> topNodes;
        std::unordered_map<std::string, std::shared_ptr<Node>> nodes;

        std::vector<vk::Sampler> samplers;

        DescriptorAllocator descriptorAllocator;
        Buffer materialBuffer;
    };

    std::optional<std::vector<std::shared_ptr<Mesh>>> loadGltfMeshes(const std::filesystem::path& filePath, commands::UploadContext& context);
    std::optional<std::shared_ptr<Mesh>> load_mesh(const std::filesystem::path& filePath, commands::UploadContext& context);

    std::optional<GLTF> load_scene_description(core::Device& device, const std::filesystem::path& filePath, commands::UploadContext& context);
    std::optional<fastgltf::Asset> load_gltf(const std::filesystem::path& filePath);

    std::optional<Image> load_image(const core::Device& device, commands::UploadContext& context, fastgltf::Asset& asset, fastgltf::Image& image);
    std::optional<ktxVulkanTexture> load_ktx(const char* path);
    std::optional<ktxVulkanTexture> load_ktx(void*, size_t size);

    void process_mesh_data(fastgltf::Asset& gltf, Mesh& mesh, const fastgltf::Mesh& fastgltfMesh, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);

    std::shared_ptr<Material> process_material(core::Device& device, commands::UploadContext& context, fastgltf::Asset& gltf, fastgltf::Material& material);
    vk::Filter extract_gltf_filter(fastgltf::Filter filter);
    vk::SamplerMipmapMode extract_mipmap_mode(fastgltf::Filter filter);

#define ktx_check(x)                    \
    do {                                \
        KTX_error_code error = x;      \
        if (error != KTX_SUCCESS) {    \
            return{};                   \
        }                               \
    } while(0)
}
*/