
#ifndef MESH_H
#define MESH_H
#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"

bool load_mesh(std::filesystem::path path) {
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None) {
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
