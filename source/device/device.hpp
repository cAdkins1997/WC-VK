
#pragma once
#include "../vkinit.h"
#include <VkBootstrap.h>

struct GraphicsContext;
struct ComputeContext;
struct RaytracingContext;
struct UploadContext;

namespace wcvk {

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class Device {

    public:
        Device();
        ~Device();

        vk::Device device;
        vk::PhysicalDevice physicalDevice;

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;

        GLFWwindow* window = VK_NULL_HANDLE;
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain = VK_NULL_HANDLE;
        vk::Format swapchainFormat;
        std::vector<vk::Image> swapchainImages{};
        std::vector<vk::ImageView> swaphchainImages{};

        vk::Queue graphics, compute, present, transfer;

        VmaAllocator allocator{};

    public:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

    };
}
