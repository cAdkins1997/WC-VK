
#ifndef DEVICE_H
#define DEVICE_H

#include <VkBootstrap.h>

#include <map>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>

#include "DeviceHelper.h"
#include "resources.h"
#include "../context.h"
#include "../pipelines/descriptors.h"

#include "InstanceHelper.h"

struct GraphicsContext;
struct ComputeContext;
struct RaytracingContext;
struct UploadContext;

typedef Buffer* VBuffer;
typedef Image* VImage;
typedef Shader* VShader;
typedef Pipeline* VPipeline;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class Device {
public:
    Device();
    ~Device();

    BufferHandle create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    TextureHandle create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped);
    VShader create_shader(const char* filePath);
    VPipeline create_pipeline(VkPipelineCreateFlagBits type);

    void submit_graphics_work(GraphicsContext& context);
    void submit_compute_work(ComputeContext& context);
    void submit_raytracing_work(RaytracingContext& context);
    void submit_upload_work(UploadContext& context);

    void wait_on_work();
    void present();

public:
    FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };
    uint32_t get_swapchain_image_index() {
        vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchainSemaphore, nullptr, &swapchainImageIndex);
        return swapchainImageIndex;
    }

public:
    uint32_t frameNumber = 0;

private:
    uint32_t swapchainImageIndex = 0;
    FrameData frames[MAX_FRAMES_IN_FLIGHT];

public:
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

private:
    InstanceHelper instanceHelper;
    DeviceHelper deviceHelper;

    VkInstance instance = instanceHelper.instance;

public:
    GLFWwindow* window = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages{};
    std::vector<VkImageView> swapChainImageViews{};

public:
    [[nodiscard]] uint32_t get_min_uniform_buffer_offset_alignment() const;

private:
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    std::vector<VkImageView> textures;
    std::vector<VkBuffer> buffers;

    TextureHandle store_texture(VkImageView imageView, VkSampler sampler);
    BufferHandle store_buffer(VkBuffer buffer, VkBufferUsageFlagBits usage);

public:
    uint32_t graphicsQueueIndex = 0, computeQueueIndex = 0, presentQueueIndex = 0;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

public:
    VmaAllocator allocator{};

public:
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

private:
    void init_commands();
    void init_sync_objects();
    void init_allocator();
    void init_descriptors();
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
        Device::~Device();                                  \
        abort();                                            \
    }                                                       \
}                                                           \
while (0);
#endif



#endif //DEVICE_H
