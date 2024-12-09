
#pragma once
#include <memory>
#include <filesystem>

#include "glmdefines.h"

#include "context.h"
#include "device/device.hpp"
#include "device/resources.h"

#include <glm/gtx/quaternion.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/base64.hpp>
#include <fastgltf/util.hpp>

#include <ktx.h>
#include <ktxvulkan.h>

#include <EASTL/optional.h>
#include <EASTL/shared_ptr.h>

namespace wcvk::meshes {

    eastl::optional<eastl::vector<eastl::shared_ptr<Mesh>>> load_gltf_meshes(const std::filesystem::path& filePath, commands::UploadContext& context);
    eastl::optional<eastl::shared_ptr<Mesh>> load_mesh(const std::filesystem::path& filePath, commands::UploadContext& context);

#define ktx_check(x)                    \
    do {                                \
        KTX_error_code error = x;      \
        if (error != KTX_SUCCESS) {    \
            return{};                   \
        }                               \
    } while(0)
}
