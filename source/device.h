
#ifndef DEVICE_H
#define DEVICE_H

#include <VkBootstrap.h>

#include <map>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>

#include "vkinit.h"
#include "context.h"

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
static constexpr bool enableValidationLayers = true;
#endif

struct GraphicsContext;
struct ComputeContext;
struct RaytracingContext;
struct UploadContext;

struct Buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct Image {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};

struct Shader {
    VkShaderModule module;
};


struct Pipeline {
    VkPipeline pipeline;
};

typedef Buffer* VBuffer;
typedef Image* VImage;
typedef Shader* VShader;
typedef Pipeline* VPipeline;

class Device {
public:
    Device();
    ~Device();

    VBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    VImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped);
    VShader create_shader(const char* filePath);
    VPipeline create_pipeline(VkPipelineCreateFlagBits type);
    void SubmitGraphicsWork(GraphicsContext& context);
    void SubmitComputeWork(ComputeContext& context);
    void SubmitRaytracingWork(RaytracingContext& context);
    void SubmitUploadWork(UploadContext& context);

    std::vector<Buffer> buffers;
    std::vector<Image> images;
    std::vector<Shader> shaders;
    std::vector<Pipeline> pipelines;

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages{};
    VkFormat swapchainImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews{};
    const uint32_t width = 1920, height = 1080;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VmaAllocator allocator;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

private:
    void init_window();
    void init_instance();
    void init_debug_messenger();
    void init_surface();
    void init_physical_device();
    void init_logical_device();
    void init_swapchain();
    void init_image_views();
    void init_allocator();

private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

private:
    bool check_validation_layer_support();
    std::vector<const char*> get_required_extensions();
    static auto debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) -> VkBool32;
    void populate_debug_messenger_CI(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VkResult create_debug_utils_messenger_ext(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    static void destroy_debug_utils_messenger_ext(VkInstance instance,
                                              VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator);

    uint32_t rate_device_suitability(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

    bool check_device_extension_support(VkPhysicalDevice device);
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
};

#ifdef NDEBUG
#define VK_CHECK(x)                                         \
x                                                           \
\

#else

#define VK_CHECK(x)                                         \
do {                                                        \
    if (x) {                                                \
        std::cerr << string_VkResult(x) ;                   \
        Device::~Device();                                          \
        abort();                                            \
    }                                                       \
}                                                           \
while (0);
#endif



#endif //DEVICE_H
