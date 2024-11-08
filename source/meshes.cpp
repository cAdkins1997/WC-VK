/*
#include "meshes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

void wcvk::meshes::Node::refreshTransform(const glm::mat4 &parentMatrix) {
    worldTransform = parentMatrix * localTransform;
    for (const auto& c : children) {
        c->refreshTransform(worldTransform);
    }
}

std::optional<std::vector<std::shared_ptr<Mesh>>> wcvk::meshes::loadGltfMeshes(
    const std::filesystem::path &filePath,
    commands::UploadContext &context
    )
{
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
    if (data.error() != fastgltf::Error::None) {
        throw std::runtime_error("Failed to read glTF file");
    }
    auto asset = parser.loadGltf(data.get(), filePath.parent_path());
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        throw std::runtime_error("failed to read GLTF buffer");
    }

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    fastgltf::Asset& gltf = asset.get();
    for (auto& mesh : asset->meshes) {
        Mesh newMesh;
        indices.clear();
        vertices.clear();
        for (auto&& primitive : mesh.primitives) {
            Surface newSurface;
            newSurface.startIndex = static_cast<uint32_t>(indices.size());
            newSurface.count = static_cast<uint32_t>(gltf.accessors[primitive.indicesAccessor.value()].count);
            size_t initialVertex = vertices.size();

            {
                fastgltf::Accessor& indexAccessor = gltf.accessors[primitive.indicesAccessor.value()];
                indices.reserve(indices.size() + indexAccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,
                    [&](std::uint32_t idx) {
                        indices.push_back(idx + initialVertex);
                    }
                );
            }

            {
                fastgltf::Accessor& posAccessor = gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                        Vertex newVerex{};
                        newVerex.position = v;
                        newVerex.normal = { 1, 0, 0 };
                        newVerex.color = glm::vec4 { 1.f };
                        newVerex.uv_x = 0;
                        newVerex.uv_y = 0;
                        vertices[initialVertex + index] = newVerex;
                    });
            }

            if (auto normals = primitive.findAttribute("NORMAL"); normals != primitive.attributes.end()) {
                auto accessorIndex = (normals)->accessorIndex;
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[accessorIndex],
                    [&](glm::vec3 v, size_t index) {
                        vertices[initialVertex + index].normal = v;
                    });
            }

            if (auto uv = primitive.findAttribute("TEXCOORD_0"); uv != primitive.attributes.end()) {
                auto accessorIndex = (uv)->accessorIndex;
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[accessorIndex],
                    [&](glm::vec2 v, size_t index) {
                        vertices[initialVertex + index].uv_x = v.x;
                        vertices[initialVertex + index].uv_y = v.y;
                    });
            }

            if (auto colors = primitive.findAttribute("COLOR_0"); colors != primitive.attributes.end()) {
                auto accessorIndex = (colors)->accessorIndex;
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[accessorIndex],
                    [&](glm::vec4 v, size_t index) {
                        vertices[initialVertex + index].color = v;
                    });
            }

            newMesh.surfaces.push_back(newSurface);
        }

        std::vector<std::shared_ptr<Material>> materials;
        for (auto& material : gltf.materials) {
            auto newMaterial = std::make_shared<Material>();
            materials.push_back(newMaterial);
        }

        if constexpr (constexpr bool OverrideColors = false) {
            for (Vertex& vtx : vertices) {
                vtx.color = glm::vec4(vtx.normal, 1.f);
            }
        }
        newMesh.mesh = context.upload_mesh(vertices, indices);

        meshes.emplace_back(std::make_shared<Mesh>(std::move(newMesh)));
    }

    return meshes;
}

std::optional<wcvk::meshes::GLTF> wcvk::meshes::load_scene_description(
    core::Device &device,
    const std::filesystem::path &filePath,
    commands::UploadContext &context)
{
    if (auto gltf = load_gltf(filePath); gltf.has_value()) {
        GLTF gltfSceneDescription;
        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<Node>> nodes;
        std::vector<Image> images;
        std::vector<std::shared_ptr<Material>> materials;

        for (auto& mesh : gltf->meshes) {
            Mesh newMesh;
            process_mesh_data(gltf.value(), newMesh, mesh, indices, vertices);

            newMesh.mesh = context.upload_mesh(vertices, indices);

            gltfSceneDescription.meshes.emplace(mesh.name, std::make_shared<Mesh>(std::move(newMesh)));
        }

        for (auto&& material : gltf.value().materials) {
            auto newMaterial = process_material(device, context, gltf.value(), material);
            gltfSceneDescription.materials.emplace(material.name.c_str(), std::move(newMaterial));
        }

        for (fastgltf::Node& node : gltf->nodes) {
            std::shared_ptr<Node> newNode;

            if (node.meshIndex.has_value()) {
                newNode = std::make_shared<MeshNode>();
                static_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
            }
            else {
                newNode = std::make_shared<Node>();
            }

            nodes.push_back(newNode);
            gltfSceneDescription.nodes[node.name.c_str()];

            const auto transformMatrix = fastgltf::getTransformMatrix(node);
            memcpy(&newNode->localTransform, transformMatrix.data(), sizeof(transformMatrix.data()));
        }

        for (int i = 0; i < gltf->nodes.size(); i++) {
            fastgltf::Node& node = gltf->nodes[i];
            std::shared_ptr<Node>& sceneNode = nodes[i];

            for (auto& c : node.children) {
                sceneNode->children.push_back(nodes[c]);
                nodes[c]->parent = sceneNode;
            }
        }

        for (auto& node : nodes) {
            if (node->parent.lock() == nullptr) {
                gltfSceneDescription.topNodes.push_back(node);
                node->refreshTransform(glm::mat4 { 1.f });
            }
        }

        return gltfSceneDescription;
    }

    return {};
}

std::optional<fastgltf::Asset> wcvk::meshes::load_gltf(const std::filesystem::path &filePath)
{
    fastgltf::Parser parser;
    fastgltf::Asset gltf;

    constexpr auto fastGLTFOptions = fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
                fastgltf::Options::LoadExternalBuffers;

    auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
    if (data.error() != fastgltf::Error::None) {
        throw std::runtime_error("Failed to read glTF file");
    }

    fastgltf::GltfDataBuffer& dataBuffer = data.get();

    if (auto type = fastgltf::determineGltfFileType(dataBuffer); type == fastgltf::GltfType::glTF) {
        auto load = parser.loadGltf(dataBuffer, filePath.parent_path(), fastGLTFOptions);
        if (load) {
            gltf = std::move(load.get());
        } else {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else if (type == fastgltf::GltfType::GLB) {
        auto load = parser.loadGltfBinary(dataBuffer, filePath.parent_path(), fastGLTFOptions);
        if (load) {
            gltf = std::move(load.get());
        } else {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else {
        std::cerr << "Failed to determine glTF container" << std::endl;
        return {};
    }

    return gltf;
}

std::shared_ptr<Material> wcvk::meshes::process_material(core::Device& device, commands::UploadContext& context, fastgltf::Asset& gltf, fastgltf::Material& material) {
    auto newMaterial = std::make_shared<Material>();

    if (material.pbrData.metallicRoughnessTexture.has_value()) {
        auto metallicRoughnessTexture = gltf.textures[material.pbrData.metallicRoughnessTexture.value().textureIndex];

        if (metallicRoughnessTexture.imageIndex.has_value() && metallicRoughnessTexture.samplerIndex.has_value()) {
            auto metalRoughnessTextureImage = gltf.images[metallicRoughnessTexture.imageIndex.value()];

            if (auto result = load_image(device, context, gltf, metalRoughnessTextureImage); result.has_value()) {
                newMaterial->mrImage = result.value();

                if (metallicRoughnessTexture.samplerIndex.has_value()) {
                    auto metallicRoughnessSampler = gltf.samplers[metallicRoughnessTexture.samplerIndex.value()];


                    auto magFilter = extract_gltf_filter(metallicRoughnessSampler.magFilter.value_or(vk::Filter::eNearest));
                    auto minFilter = extract_gltf_filter(metallicRoughnessSampler.minFilter.value_or(vk::Filter::eNearest));
                    auto mipmapMode = extract_mipmap_mode(metallicRoughnessSampler.minFilter.value_or(vk::Filter::eNearest));

                    newMaterial->mrImage.sampler = device.create_sampler(minFilter, magFilter, mipmapMode);
                }
            }
        }
    }

    if (material.pbrData.baseColorTexture.has_value()) {
        auto baseColorTexture = gltf.textures[material.pbrData.baseColorTexture.value().textureIndex];

        if (baseColorTexture.imageIndex.has_value() && baseColorTexture.samplerIndex.has_value()) {
            auto baseColorTextureImage = gltf.images[baseColorTexture.imageIndex.value()];

            if (auto result = load_image(device, context, gltf, baseColorTextureImage); result.has_value()) {
                newMaterial->colorImage = result.value();

                if (baseColorTexture.samplerIndex.has_value()) {
                    auto baseColorSampler = gltf.samplers[baseColorTexture.samplerIndex.value()];

                    auto magFilter = extract_gltf_filter(baseColorSampler.magFilter.value_or(vk::Filter::eNearest));
                    auto minFilter = extract_gltf_filter(baseColorSampler.minFilter.value_or(vk::Filter::eNearest));
                    auto mipmapMode = extract_mipmap_mode(baseColorSampler.minFilter.value_or(vk::Filter::eNearest));

                    newMaterial->colorImage.sampler = device.create_sampler(minFilter, magFilter, mipmapMode);
                }
            }
        }
    }

    newMaterial->dataBuffer = device.create_buffer(sizeof(Material::MatConstants) * gltf.materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto* sceneDataUniformBuffer = static_cast<Material::MatConstants*>(newMaterial->dataBuffer.get_mapped_data());
    sceneDataUniformBuffer->mrFactors = glm::vec4{1,1,1,1};
    sceneDataUniformBuffer->baseColorFactors = glm::vec4{1,0.5,0,0};


    return newMaterial;
}

vk::Filter wcvk::meshes::extract_gltf_filter(const fastgltf::Filter filter) {
    switch (filter) {
        case fastgltf::Filter::Nearest:
        case fastgltf::Filter::NearestMipMapNearest:
        case fastgltf::Filter::NearestMipMapLinear:
            return vk::Filter::eNearest;

        case fastgltf::Filter::Linear:
        case fastgltf::Filter::LinearMipMapNearest:
        case fastgltf::Filter::LinearMipMapLinear:
        default:
            return vk::Filter::eLinear;
    }
}

vk::SamplerMipmapMode wcvk::meshes::extract_mipmap_mode(fastgltf::Filter filter) {
    switch (filter) {
        case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::LinearMipMapNearest:
            return vk::SamplerMipmapMode::eNearest;

        case fastgltf::Filter::NearestMipMapLinear:
        case fastgltf::Filter::LinearMipMapLinear:
        default:
            return vk::SamplerMipmapMode::eLinear;
    }
}

std::optional<ktxVulkanTexture> wcvk::meshes::load_ktx(const char* path) {
    ktxTexture* kTexture;
    ktxVulkanDeviceInfo vdi;
    ktxVulkanTexture texture;

    //ktxVulkanDeviceInfo_Construct(&vdi, ctx.physicalDevice, ctx.device, ctx.queue, ctx.commandPool, nullptr);

    ktx_check(ktxTexture_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture));
    ktx_check(ktxTexture_VkUploadEx(kTexture, &vdi, &texture, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    ktxTexture_Destroy(kTexture);
    ktxVulkanDeviceInfo_Destruct(&vdi);

    return texture;
}

std::optional<ktxVulkanTexture> wcvk::meshes::load_ktx(void* data, size_t size) {
    ktxTexture* kTexture;
    ktxVulkanDeviceInfo vdi;
    ktxVulkanTexture texture;

    //ktxVulkanDeviceInfo_Construct(&vdi, ctx.physicalDevice, ctx.device, ctx.queue, ctx.commandPool, nullptr);

    auto ktxData = static_cast<ktx_uint8_t*>(data);

    ktx_check(ktxTexture_CreateFromMemory(ktxData, size, KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture));
    ktx_check(ktxTexture_VkUploadEx(kTexture, &vdi, &texture, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    ktxTexture_Destroy(kTexture);
    ktxVulkanDeviceInfo_Destruct(&vdi);

    return texture;
}

std::optional<Image> wcvk::meshes::load_image(const core::Device& device, commands::UploadContext& context, fastgltf::Asset &asset, fastgltf::Image& image) {
    Image newImage{};

    int width, height, nrChannels;

        std::visit(
            fastgltf::visitor {
            [](auto& arg) {},
            [&](const fastgltf::sources::URI& filePath) {

                const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());

                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imageSize;
                    imageSize.width = width;
                    imageSize.height = height;
                    imageSize.depth = 1;

                    newImage = device.create_image(imageSize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);
                    context.upload_image(data, newImage);

                    stbi_image_free(data);
                }

                /*if (auto result = load_ktx(path.data(), ctx); result.has_value()) {
                    ktxVulkanTexture texture = result.value();
                    vk::Extent3D extent {texture.width, texture.height, texture.depth};
                    newImage = device.create_image(extent, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, true);
                    newImage.image = result.value().image;
                }
            },
            [&](fastgltf::sources::Vector& vector) {
                unsigned char* data  = stbi_load_from_memory(
                    reinterpret_cast<stbi_uc*>(vector.bytes.data()),
                    static_cast<int>(vector.bytes.size()),
                    &width,
                    &height,
                    &nrChannels,
                    4);
                if (data) {
                    VkExtent3D imageSize;
                    imageSize.width = width;
                    imageSize.height = height;
                    imageSize.depth = 1;

                    newImage = device.create_image(imageSize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);
                    context.upload_image(data, newImage);

                    stbi_image_free(data);
                }

                /*if (auto result = load_ktx(vector.bytes.data(), ctx, vector.bytes.size()); result.has_value()) {
                    ktxVulkanTexture texture = result.value();
                    vk::Extent3D extent {texture.width, texture.height, texture.depth};
                    newImage = device.create_image(extent, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, true);
                }
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                std::visit(fastgltf::visitor {
                               [](auto& arg) {},
                               [&](fastgltf::sources::Array& vector) {
                                   unsigned char* data = stbi_load_from_memory(
                                       reinterpret_cast<stbi_uc *>(vector.bytes.data()) + bufferView.byteOffset,
                                      static_cast<int>(bufferView.byteLength),
                                      &width, &height, &nrChannels, 4);
                                   if (data) {
                                       VkExtent3D imageSize;
                                       imageSize.width = width;
                                       imageSize.height = height;
                                       imageSize.depth = 1;

                                       newImage = device.create_image(imageSize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, true);
                                       context.upload_image(data, newImage);

                                       stbi_image_free(data);
                                   }

                                   /*if (auto result = load_ktx(vector.bytes.data() + bufferView.byteOffset, ctx, bufferView.byteLength); result.has_value()) {
                                       ktxVulkanTexture texture = result.value();
                                       vk::Extent3D extent {texture.width, texture.height, texture.depth};
                                       newImage = device.create_image(extent, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eSampled, true);
                                   }
                               }
                }, buffer.data);
            },
        },
        image.data);

    if (newImage.image == VK_NULL_HANDLE) {
        return {};
    }

    return newImage;
}

void wcvk::meshes::process_mesh_data(fastgltf::Asset& gltf,  Mesh& mesh, const fastgltf::Mesh& fastgltfMesh, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices) {
    indices.clear();
    vertices.clear();
    for (auto&& primitive : fastgltfMesh.primitives) {
        Surface newSurface;
        newSurface.startIndex = static_cast<uint32_t>(indices.size());
        newSurface.count = static_cast<uint32_t>(gltf.accessors[primitive.indicesAccessor.value()].count);
        size_t initialVertex = vertices.size();

        {
            fastgltf::Accessor& indexAccessor = gltf.accessors[primitive.indicesAccessor.value()];
            indices.reserve(indices.size() + indexAccessor.count);

            fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,
                [&](std::uint32_t idx) {
                    indices.push_back(idx + initialVertex);
                }
            );
        }

        {
            fastgltf::Accessor& posAccessor = gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex];
            vertices.resize(vertices.size() + posAccessor.count);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                [&](glm::vec3 v, size_t index) {
                    Vertex newVertex{};
                    newVertex.position = v;
                    newVertex.normal = { 1, 0, 0 };
                    newVertex.color = glm::vec4 { 1.f };
                    newVertex.uv_x = 0;
                    newVertex.uv_y = 0;
                    vertices[initialVertex + index] = newVertex;
                });
        }

        if (auto normals = primitive.findAttribute("NORMAL"); normals != primitive.attributes.end()) {
            auto accessorIndex = (normals)->accessorIndex;
            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[accessorIndex],
                [&](glm::vec3 v, size_t index) {
                    vertices[initialVertex + index].normal = v;
                });
        }

        if (auto uv = primitive.findAttribute("TEXCOORD_0"); uv != primitive.attributes.end()) {
            auto accessorIndex = (uv)->accessorIndex;
            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[accessorIndex],
                [&](glm::vec2 v, size_t index) {
                    vertices[initialVertex + index].uv_x = v.x;
                    vertices[initialVertex + index].uv_y = v.y;
                });
        }

        if (auto colors = primitive.findAttribute("COLOR_0"); colors != primitive.attributes.end()) {
            auto accessorIndex = (colors)->accessorIndex;
            fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[accessorIndex],
                [&](glm::vec4 v, size_t index) {
                    vertices[initialVertex + index].color = v;
                });
        }

        mesh.surfaces.push_back(newSurface);
    }
}
*/