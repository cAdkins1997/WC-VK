
#pragma once
#include "resources.h"
#include "../context.h"
#include "../vkcommon.h"
#include "../camera.h"

#include <fstream>
#include <VkBootstrap.h>
#include <VkBootstrapDispatch.h>
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <span>
#include <functional>
#include <filesystem>

namespace wcvk::commands {
    struct GraphicsContext;
    struct ComputeContext;
    struct RaytracingContext;
    struct UploadContext;
}

namespace wcvk::core {
    inline Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

    void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void process_input(GLFWwindow *window);

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class Device {

    public:
        [[nodiscard]] Buffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) const;
        [[nodiscard]] Image* create_image(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped);
        [[nodiscard]] Shader create_shader(std::string_view filePath) const;

        void submit_graphics_work(const commands::GraphicsContext& context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal);
        void submit_compute_work(const commands::ComputeContext& context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal);
        void submit_raytracing_work(commands::RaytracingContext& context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal);
        void submit_upload_work(const commands::UploadContext& context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal);
        void submit_upload_work(const commands::UploadContext& context);

        void submit_immediate_work(std::function<void(VkCommandBuffer cmd)>&& function);

        void wait_on_work();
        void present();

        Image& get_draw_image() { return drawImage; }
        Image& get_depth_image() { return depthImage; }
        VkImage& get_swapchain_image();

    public:
        Device();
        ~Device();

        [[nodiscard]] vk::Device& get_handle() { return device; };

        vk::Device device;
        vk::PhysicalDevice physicalDevice;

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;

        uint32_t width = 2560, height = 1440;
        GLFWwindow* window;
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::Format swapchainFormat;
        std::vector<VkImage> swapchainImages{};
        std::vector<VkImageView> swapchainImageViews{};
        vk::Extent2D swapchainExtent{};
        uint32_t swapchainImageIndex = 0;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        uint32_t frameNumber = 0;
        bool resizeRequested = false;

        Image depthImage;
        Image drawImage;

        uint32_t graphicsQueueIndex, computeQueueIndex, presentQueueIndex{}, transferQueueIndex;
        vk::Queue graphicsQueue, computeQueue, presentQueue, transferQueue;

        VmaAllocator allocator{};

        std::vector<Image> images;

        vk::Fence immediateFence;
        vk::CommandPool immediateCommandPool;
        vk::CommandBuffer immediateCommandBuffer;

        FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };
        uint32_t get_swapchain_image_index() {
            if (device.acquireNextImageKHR(swapchain, 1000000000, get_current_frame().swapchainSemaphore, nullptr, &swapchainImageIndex) == vk::Result::eErrorOutOfDateKHR) {
                resizeRequested = true;
            }

            return swapchainImageIndex;
        }

    public:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        DeletionQueue primaryDeletionQueue;

    private:
        void init_commands();
        void init_sync_objects();
        void init_allocator();
        void init_descriptors();
        void init_draw_images();
        void init_depth_images();
    };
}
