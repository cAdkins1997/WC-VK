
#ifndef MESH_H
#define MESH_H

#include <bits/fs_path.h>
#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"

#include "../vkinit.h"
#include "../device/resources.h"

MeshBuffer upload_mesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);


}

bool load_mesh(std::filesystem::__cxx11::path path) {
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::loadFromFile(path);
    if (data != fastgltf::Error::None) {
        return false;
    }

    auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        return false;
    }

    for (auto& buffer : asset->buffers) {

    }

    return false;
}

#endif //MESH_H
