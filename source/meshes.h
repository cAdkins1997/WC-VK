
#ifndef MESHES_H
#define MESHES_H

#include <memory>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/base64.hpp>
#include <fastgltf/util.hpp>

#include "context.h"
#include "device/resources.h"

namespace wcvk::meshes {
    std::optional<std::vector<std::shared_ptr<Mesh>>> loadGltfMeshes(const std::filesystem::path& filePath, commands::UploadContext& context);
}

#endif //MESHES_H
