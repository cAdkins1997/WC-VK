#pragma once
#include "glmdefines.h"
#include "vkcommon.h"
#include <GLFW/glfw3.h>

namespace wcvk {
    class Camera;

    inline float lastX = 1920 / 2.0f;
    inline float lastY = 1080 / 2.0f;
    inline bool firstMouse = true;
    inline float deltaTime = 0.0f;
    inline float lastFrameTime = 0.0f;

    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float SPEED = 2.5f;
    constexpr float SENSITIVITY = 0.1f;
    constexpr float ZOOM =  45.0f;

    class Camera {
    public:
        glm::vec3 Position{};
        glm::vec3 Front;
        glm::vec3 Up{};
        glm::vec3 Right{};
        glm::vec3 WorldUp{};
        float yaw;
        float pitch;
        float movementSpeed;
        float mouseSensitivity;
        float zoom;

        explicit Camera(
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float _yaw = YAW,
            float _pitch = PITCH
            );

        Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float _yaw, float _pitch);

        [[nodiscard]] glm::mat4 get_view_matrix() const;

        void process_mouse_scroll(float yoffset);
        void process_mouse_movement(float xoffset, float yoffset, bool constrainPitch);
        void process_keyboard(Camera_Movement direction, float deltaTime);

    private:
        void update_camera_vectors();
    };
}