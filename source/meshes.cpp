
#include "meshes.h"

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
        newMesh.name = mesh.name;
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
