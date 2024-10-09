#pragma once
#include <glm/glm.hpp>
#include <array>

namespace wcvk {
    typedef glm::vec4 Plane;
    typedef std::array<Plane, 6> Frustum;

    Frustum compute_frustum(glm::mat4 matrix);
}
