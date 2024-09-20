
#pragma once
#include "resources.h"
#include "../context.h"
#include "../vkcommon.h"
#include <fstream>
#include <VkBootstrap.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/optional.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/span.h>
#include <fastgltf/core.hpp>

namespace wcvk::commands {
    struct GraphicsContext;
    struct ComputeContext;
    struct RaytracingContext;
    struct UploadContext;
}

namespace wcvk::core {

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class Device {

    public:
        [[nodiscard]] Buffer create_buffer(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage) const;
        [[nodiscard]] Image* create_image(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped);
        Shader create_shader(const char* filePath) const;

        void submit_graphics_work(const commands::GraphicsContext& context);
        void submit_compute_work(const commands::ComputeContext& context);
        void submit_raytracing_work(commands::RaytracingContext& context);
        void submit_upload_work(commands::UploadContext& context);

        void wait_on_work();
        void present();

        VkImage& get_swapchain_image();
        VkImage& get_draw_image();

    public:
        Device();
        ~Device();

        vk::Device device;
        vk::PhysicalDevice physicalDevice;

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;

        uint32_t width = 1920, height = 1080;
        GLFWwindow* window;
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::Format swapchainFormat;
        eastl::vector<VkImage> swapchainImages{};
        eastl::vector<VkImageView> swapchainImageViews{};
        vk::Extent2D swapchainExtent{};
        uint32_t swapchainImageIndex = 0;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];
        uint32_t frameNumber = 0;

        Image depthImage;
        Image drawImage;

        uint32_t graphicsQueueIndex, computeQueueIndex, presentQueueIndex{}, transferQueueIndex;
        vk::Queue graphicsQueue, computeQueue, presentQueue, transferQueue;

        VmaAllocator allocator{};

        eastl::vector<Image> images;

        FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };
        uint32_t get_swapchain_image_index() {
            vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchainSemaphore, nullptr, &swapchainImageIndex);
            return swapchainImageIndex;
        }

    public:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;


    private:
        void init_commands();
        void init_sync_objects();
        void init_allocator();
        void init_descriptors();
        void init_draw_images();;
    };
}
