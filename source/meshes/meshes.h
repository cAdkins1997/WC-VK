
#pragma once
#include "../device/device.hpp"
#include "../context.h"

#include <EASTL/vector.h>
#include <EASTL/optional.h>
#include <EASTL/shared_ptr.h>

namespace wcvk::meshes {
    eastl::optional<eastl::vector<eastl::shared_ptr<Mesh>>> load_GLTF_meshs(core::Device& device, commands::UploadContext& context,  const char* path);
    MeshBuffer upload_mesh(core::Device& device, eastl::span<uint32_t> indices, eastl::span<Vertex> vertices);
}
