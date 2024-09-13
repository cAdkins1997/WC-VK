
#pragma once
#include "resources.h"
#include "../context.h"
#include "../vkcommon.h"
#include <VkBootstrap.h>
#include <EASTL/vector.h>

namespace wcvk {

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class Device {

    public:
        void submit_graphics_work(const GraphicsContext& context);
        void submit_compute_work(ComputeContext& context);
        void submit_raytracing_work(RaytracingContext& context);

        void wait_on_work();
        void present();

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
        void init_draw_images();
    };
}
