
#include "scenedesc.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

void wcvk::GLTF::Material::build_pipelines(core::Device *device, vk::DescriptorSetLayout gpuSceneDataDescriptorLayout) {
    Shader vertexShader = device->create_shader("shaders/meshbufferBDA.vert.spv");
    Shader fragShader = device->create_shader("shaders/triangle.frag.spv");

    vk::PushConstantRange matrixRange{};
    matrixRange.offset = 0;
    matrixRange.size = sizeof(PushConstants);
    matrixRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

    DescriptorLayoutBuilder layoutBuilder;
    layoutBuilder.add_binding(0, vk::DescriptorType::eUniformBuffer);
    layoutBuilder.add_binding(1, vk::DescriptorType::eCombinedImageSampler);
    layoutBuilder.add_binding(2, vk::DescriptorType::eCombinedImageSampler);

    materialLayout = layoutBuilder.build(
        device->get_handle(),
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    vk::DescriptorSetLayout layouts[] = { gpuSceneDataDescriptorLayout, materialLayout };

    vk::PipelineLayoutCreateInfo meshLayoutInfo;
    meshLayoutInfo.setLayoutCount = 2;
    meshLayoutInfo.pSetLayouts = layouts;
    meshLayoutInfo.pPushConstantRanges = &matrixRange;
    meshLayoutInfo.pushConstantRangeCount = 1;

    vk::Device dHandle = device->get_handle();
    vk::PipelineLayout newLayout = dHandle.createPipelineLayout(meshLayoutInfo, nullptr);

    opaquePipeline.layout = newLayout;
    transparentPipeline.layout = newLayout;

    PipelineBuilder pipelineBuilder;
    pipelineBuilder.set_shader(vertexShader.module, fragShader.module);
    pipelineBuilder.set_input_topology(vk::PrimitiveTopology::eTriangleList);
    pipelineBuilder.set_polygon_mode(vk::PolygonMode::eFill);
    pipelineBuilder.set_cull_mode(vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
    pipelineBuilder.set_multisampling_none();
    pipelineBuilder.disable_blending();
    pipelineBuilder.enable_depthtest(true, vk::CompareOp::eGreaterOrEqual);
    pipelineBuilder.set_color_attachment_format(device->drawImage.imageFormat);
    pipelineBuilder.set_depth_format(device->depthImage.imageFormat);

    pipelineBuilder.pipelineLayout = newLayout;
    opaquePipeline.pipeline = pipelineBuilder.build_pipeline(device->get_handle());
    pipelineBuilder.enable_blending_additive();
    pipelineBuilder.enable_depthtest(false, vk::CompareOp::eGreaterOrEqual);
    transparentPipeline.pipeline = pipelineBuilder.build_pipeline(device->get_handle());
    vkDestroyShaderModule(device->get_handle(), fragShader.module, nullptr);
    vkDestroyShaderModule(device->get_handle(), vertexShader.module, nullptr);

}

void wcvk::GLTF::Material::clear_resources(vk::Device device) {

}

MaterialInstance wcvk::GLTF::Material::write_material(
     vk::Device device,
     MaterialPass pass,
    const MaterialResources &resources,
    DescriptorAllocator &descriptorAllocator)
{
    MaterialInstance matData;
    matData.passType = pass;
    if (pass == MaterialPass::Transparent) {
        matData.pipeline = &transparentPipeline;
    }
    else {
        matData.pipeline = &opaquePipeline;
    }

    matData.materialSet = descriptorAllocator.allocate(device, materialLayout);


    writer.clear();
    writer.write_buffer(0, resources.dataBuffer, sizeof(MatConstants), resources.dataBufferOffset, vk::DescriptorType::eUniformBuffer);
    writer.write_image(1, resources.colorImage.imageView, resources.colorSampler, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
    writer.write_image(2, resources.metalRoughImage.imageView, resources.metalRoughSampler, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eCombinedImageSampler);
    writer.update_set(device, matData.materialSet);

    return matData;
}

void wcvk::GLTF::Node::refreshTransform(const glm::mat4 &parentMatrix) {
    worldTransform = parentMatrix * localTransform;
    for (const auto& c : children) {
        c->refreshTransform(worldTransform);
    }
}

void wcvk::GLTF::Node::Draw(const glm::mat4 &topMatrix, DrawContext &ctx) {
    for (auto& c : children) {
        c->Draw(topMatrix, ctx);
    }
}

void wcvk::GLTF::MeshNode::Draw(const glm::mat4 &topMatrix, DrawContext &ctx) {
    const glm::mat4 nodeMatrix = topMatrix * worldTransform;

    for (auto& surface : mesh->surfaces) {
        RenderObject def{};
        def.indexCount = surface.count;
        def.firstIndex = surface.startIndex;
        def.indexBuffer = mesh->mesh.indexBuffer.buffer;
        def.material = &surface.material->data;
        def.transform = nodeMatrix;
        def.vertexBufferAddress = mesh->mesh.deviceAddress;

        ctx.OpaqueSurfaces.push_back(def);
    }


    Node::Draw(topMatrix, ctx);
}

void wcvk::GLTF::LoadedGLTF::Draw(const glm::mat4 &topMatrix, DrawContext &ctx) {
    for (auto& n : topNodes) {
        n->Draw(topMatrix, ctx);
    }
}

void wcvk::GLTF::LoadedGLTF::clearAll() {
    vk::Device device = creator->get_handle();

    descriptorPool.destroy_pools(device);
    device.destroyBuffer(materialDataBuffer.buffer);

    for (auto& [k, v] : meshes) {
        device.destroyBuffer(v->mesh.indexBuffer.buffer);
        device.destroyBuffer(v->mesh.vertexBuffer.buffer);
    }

    for (auto& [k, v] : images) {

        if (v.image == checkerboardImage.image) {
            continue;
        }

        device.destroyImage(v.get_handle());
    }

    for (auto& sampler : samplers) {
        device.destroySampler(sampler, nullptr);
    }
}

std::optional<std::shared_ptr<wcvk::GLTF::LoadedGLTF>> wcvk::GLTF::load_GLTF(
    core::Device &device,
    commands::UploadContext& uploadContext,
    std::string_view filepath,
    Material& metalRoughMaterial)
{
    auto scene = std::make_shared<LoadedGLTF>();

    scene->creator = &device;
    LoadedGLTF& file = *scene;

    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
    uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));

    std::array<uint32_t, 16 * 16 > pixels{};
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }

    file.checkerboardImage = device.create_image({ 16, 16, 1}, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);
    file.whiteImage = device.create_image({ 16, 16, 1}, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);

    uploadContext.upload_image(pixels.data(), file.checkerboardImage);
    uploadContext.upload_image(&white, file.whiteImage);

    fastgltf::Parser parser {};

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember
                                | fastgltf::Options::AllowDouble
                                | fastgltf::Options::LoadExternalBuffers
                                | fastgltf::Options::DecomposeNodeMatrices;
    fastgltf::GltfDataBuffer data;

    fastgltf::GltfDataBuffer::FromPath(filepath);

    fastgltf::Asset gltf;

    const std::filesystem::path path = filepath;

    if (auto type = determineGltfFileType(data); type == fastgltf::GltfType::glTF) {
        auto load = parser.loadGltf(data, path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        } else {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    } else if (type == fastgltf::GltfType::GLB) {
        auto load = parser.loadGltf(data, path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        } else {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    } else {
        std::cerr << "Failed to determine glTF container" << std::endl;
        return {};
    }

    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
        {
        { vk::DescriptorType::eCombinedImageSampler, 3 },
        { vk::DescriptorType::eUniformBuffer, 3 },
        { vk::DescriptorType::eStorageBuffer, 1 }
        };

    file.descriptorPool.init(device.get_handle(), gltf.materials.size(), sizes);

    for (fastgltf::Sampler& sampler : gltf.samplers) {

        vk::Filter magFilter = extract_filter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
        vk::Filter minFilter = extract_filter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));
        vk::SamplerMipmapMode mipMapMode = extract_mipmap_mode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        vk::Sampler newSampler = device.create_sampler(minFilter, magFilter, mipMapMode);
        file.samplers.push_back(newSampler);
    }

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<Image> images;
    std::vector<std::shared_ptr<GLTFMaterial>> materials;

    for (fastgltf::Image& image : gltf.images) {
        if (std::optional<Image> img = load_image(device, uploadContext, gltf, image); img.has_value()) {
            images.push_back(*img);
            file.images[image.name.c_str()] = *img;
        }
        else {
            images.push_back(file.checkerboardImage);
            std::cout << "gltf failed to load texture " << image.name << std::endl;
        }
    }

    file.materialDataBuffer = device.create_buffer(
        sizeof(Material::MatConstants) * gltf.materials.size(),
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VMA_MEMORY_USAGE_CPU_TO_GPU);
    int data_index = 0;
    auto sceneMaterialConstants = static_cast<Material::MatConstants*>(file.materialDataBuffer.info.pMappedData);

    for (fastgltf::Material& mat : gltf.materials) {
        auto newMat = std::make_shared<GLTFMaterial>();
        materials.push_back(newMat);
        file.materials[mat.name.c_str()] = newMat;

        Material::MatConstants constants;
        constants.baseColorFactors = {
            mat.pbrData.baseColorFactor[0],
            mat.pbrData.baseColorFactor[1],
            mat.pbrData.baseColorFactor[2],
            mat.pbrData.baseColorFactor[3]
        };

        constants.mrFactors = {mat.pbrData.metallicFactor, mat.pbrData.roughnessFactor, 0.0f, 0.0f };
        sceneMaterialConstants[data_index] = constants;

        auto passType = MaterialPass::MainColor;
        if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
            passType = MaterialPass::Transparent;
        }

        Material::MaterialResources materialResources;
        materialResources.colorImage = file.checkerboardImage;
        materialResources.colorSampler = device.drawImageSamplerLinear;
        materialResources.metalRoughImage = file.checkerboardImage;
        materialResources.metalRoughSampler = device.drawImageSamplerLinear;

        materialResources.dataBuffer = file.materialDataBuffer.buffer;
        materialResources.dataBufferOffset = data_index * sizeof(Material::MatConstants);

        if (mat.pbrData.baseColorTexture.has_value()) {
            size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
            size_t sampler = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();

            materialResources.colorImage = images[img];
            materialResources.colorSampler = file.samplers[sampler];
        }


        newMat->data = metalRoughMaterial.write_material(device.get_handle(), passType, materialResources, file.descriptorPool);
        data_index++;
    }

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& mesh : gltf.meshes) {
        auto newmesh = std::make_shared<Mesh>();
        meshes.push_back(newmesh);
        file.meshes[mesh.name.c_str()] = newmesh;
        newmesh->name = mesh.name;

        indices.clear();
        vertices.clear();

        for (auto&& primitive : mesh.primitives) {
            Surface newSurface;
            newSurface.startIndex = static_cast<uint32_t>(indices.size());
            newSurface.count = static_cast<uint32_t>(gltf.accessors[primitive.indicesAccessor.value()].count);

            size_t initial_vtx = vertices.size();

            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[primitive.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                    });
            }

            {
                fastgltf::Accessor& posAccessor = gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                        Vertex newvtx{};
                        newvtx.position = v;
                        newvtx.normal = { 1, 0, 0 };
                        newvtx.color = glm::vec4 { 1.f };
                        newvtx.uv_x = 0;
                        newvtx.uv_y = 0;
                        vertices[initial_vtx + index] = newvtx;
                    });
            }

            if (auto normals = primitive.findAttribute("NORMAL"); normals != primitive.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->accessorIndex],
                    [&](glm::vec3 v, size_t index) {
                        vertices[initial_vtx + index].normal = v;
                    });
            }

            if (auto uv = primitive.findAttribute("TEXCOORD_0"); uv != primitive.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv->accessorIndex],
                    [&](glm::vec2 v, size_t index) {
                        vertices[initial_vtx + index].uv_x = v.x;
                        vertices[initial_vtx + index].uv_y = v.y;
                    });
            }

            if (auto colors = primitive.findAttribute("COLOR_0"); colors != primitive.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->accessorIndex],
                    [&](glm::vec4 v, size_t index) {
                        vertices[initial_vtx + index].color = v;
                    });
            }

            if (primitive.materialIndex.has_value()) {
                newSurface.material = materials[primitive.materialIndex.value()];
            } else {
                newSurface.material = materials[0];
            }

            newmesh->surfaces.push_back(newSurface);
        }

        newmesh->mesh = uploadContext.upload_mesh(vertices, indices);
    }

    for (fastgltf::Node& node : gltf.nodes) {
        std::shared_ptr<Node> newNode;

        if (node.meshIndex.has_value()) {
            newNode = std::make_shared<MeshNode>();
            dynamic_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
        } else {
            newNode = std::make_shared<Node>();
        }

        nodes.push_back(newNode);
        file.nodes[node.name.c_str()];

        std::visit(fastgltf::visitor { [&](fastgltf::math::dmat4x4 matrix) {
                                          memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                                      },
                       [&](fastgltf::TRS transform) {
                           glm::vec3 tl(transform.translation[0], transform.translation[1],
                               transform.translation[2]);
                           glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
                               transform.rotation[2]);
                           glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                           glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                           glm::mat4 rm = glm::toMat4(rot);
                           glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                           newNode->localTransform = tm * rm * sm;
                       }
        },
            node.transform);
    }

    for (int i = 0; i < gltf.nodes.size(); i++) {
        fastgltf::Node& node = gltf.nodes[i];
        std::shared_ptr<Node>& sceneNode = nodes[i];

        for (auto& c : node.children) {
            sceneNode->children.push_back(nodes[c]);
            nodes[c]->parent = sceneNode;
        }
    }

    for (auto& node : nodes) {
        if (node->parent.lock() == nullptr) {
            file.topNodes.push_back(node);
            node->refreshTransform(glm::mat4 { 1.f });
        }
    }

    return scene;
}

std::optional<Image> load_image(
    wcvk::core::Device& device,
    wcvk::commands::UploadContext& uploadContext,
    fastgltf::Asset& asset,
    fastgltf::Image& image)
{
    Image newImage {};

    int width, height, nrChannels;

    std::visit(
        fastgltf::visitor {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                                                    // local files.

                const std::string path(filePath.uri.path().begin(),
                    filePath.uri.path().end()); // Thanks C++.
                if (unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4)) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    newImage = device.create_image(imagesize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);
                    uploadContext.upload_image(data, newImage);

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::Vector& vector) {
                unsigned char* data = stbi_load_from_memory(
                    reinterpret_cast<stbi_uc*>(vector.bytes.data()),
                    static_cast<int>(vector.bytes.size()),
                    &width,
                    &height,
                    &nrChannels,
                    4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    newImage = device.create_image(imagesize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);
                    uploadContext.upload_image(data, newImage);

                    stbi_image_free(data);
                }
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                std::visit(fastgltf::visitor {
                               [](auto& arg) {},
                               [&](fastgltf::sources::Vector& vector) {
                                   unsigned char* data = stbi_load_from_memory(
                                       reinterpret_cast<stbi_uc*>(vector.bytes.data() + bufferView.byteOffset),
                                       static_cast<int>(bufferView.byteLength),
                                       &width,
                                       &height,
                                       &nrChannels,
                                       4);
                                   if (data) {
                                       VkExtent3D imagesize;
                                       imagesize.width = width;
                                       imagesize.height = height;
                                       imagesize.depth = 1;

                                       newImage = device.create_image(imagesize, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled, false);

                                       stbi_image_free(data);
                                   }
                               } },
                    buffer.data);
            },
        },
        image.data);

    if (newImage.image == VK_NULL_HANDLE) {
        return {};
    } else {
        return newImage;
    }
}

vk::Filter wcvk::GLTF::extract_filter(fastgltf::Filter filter) {
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

vk::SamplerMipmapMode wcvk::GLTF::extract_mipmap_mode(fastgltf::Filter filter) {
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
