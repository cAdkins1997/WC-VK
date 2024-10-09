
#include "frustum.h"

wcvk::Frustum wcvk::compute_frustum(glm::mat4 matrix) {
    Plane left {
        matrix[0][3] + matrix[0][0],
        matrix[1][3] + matrix[1][0],
        matrix[2][3] + matrix[2][0],
        matrix[3][3] + matrix[3][0]
    };

    Plane right {
        matrix[0][3] - matrix[0][0],
        matrix[1][3] - matrix[1][0],
        matrix[2][3] - matrix[2][0],
        matrix[3][3] - matrix[3][0]
    };

    Plane bottom {
        matrix[0][3] + matrix[0][1],
        matrix[1][3] + matrix[1][1],
        matrix[2][3] + matrix[2][1],
        matrix[3][3] + matrix[3][1]
    };

    Plane top {
        matrix[0][3] - matrix[0][1],
        matrix[1][3] - matrix[1][1],
        matrix[2][3] - matrix[2][1],
        matrix[3][3] - matrix[3][1]
    };

    Plane near {
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2]
    };

    Plane far {
        matrix[0][3] - matrix[0][2],
        matrix[1][3] - matrix[1][2],
        matrix[2][3] - matrix[2][2],
        matrix[3][3] - matrix[3][2]
    };

    return {left, right, bottom, top, near, far};
}