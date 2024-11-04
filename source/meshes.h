
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

    struct MeshData {
        uint32_t meshIndex;
        uint32_t meshPrimitiveIndex;
        uint32_t baseVertexOffset;
        uint32_t baseIndexOffset;
        uint32_t NumVertices;
        uint32_t NumIndices;
    };

    std::optional<std::vector<std::shared_ptr<Mesh>>> loadGltfMeshes(const std::filesystem::path& filePath, commands::UploadContext& context);
    std::optional<std::shared_ptr<Mesh>> load_mesh(const std::filesystem::path& filePath, commands::UploadContext& context);

    std::optional<SceneDescriptionData> load_scene_description(core::Device& device, const std::filesystem::path& filePath, commands::UploadContext& context, Buffer& materialUniform);
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
