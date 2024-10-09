#include "camera.h"

#include <glm/ext/matrix_transform.hpp>

namespace wcvk {
    Camera::Camera(glm::vec3 position, glm::vec3 up, float _yaw, float _pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        yaw = _yaw;
        pitch = _pitch;
        update_camera_vectors();
    }

    Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float _yaw, float _pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        yaw = _yaw;
        pitch = _pitch;
        update_camera_vectors();
    }

    glm::mat4 Camera::get_view_matrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void Camera::process_keyboard(Camera_Movement direction, float deltaTime)  {
        float velocity = movementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    void Camera::process_mouse_movement(float xoffset, float yoffset, bool constrainPitch)  {
        xoffset *= movementSpeed;
        yoffset *= movementSpeed;

        yaw += xoffset;
        pitch -= yoffset;

        if (constrainPitch)
        {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        update_camera_vectors();
    }

    void Camera::process_mouse_scroll(float yoffset) {
        zoom -= yoffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

    void Camera::update_camera_vectors() {
        glm::vec3 front;
        front.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
        front.y = sinf(glm::radians(pitch));
        front.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
}
