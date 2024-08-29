
#ifndef DEVICEHELPER_H
#define DEVICEHELPER_H

#include <set>
#include <map>

#include "InstanceHelper.h"
#include "../vkinit.h"

class DeviceHelper {
public:
    explicit DeviceHelper(InstanceHelper& instanceHelper);
    ~DeviceHelper();

    void assign_queue_family_indices(uint32_t& graphics, uint32_t& compute, uint32_t& present) const;

    InstanceHelper& instanceHelper;

    VkInstance& instance;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages{};
    std::vector<VkImageView> swapChainImageViews{};
    VkFormat swapchainImageFormat{};
    VkExtent2D swapChainExtent{};
    const uint32_t width = 1920, height = 1080;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device) const;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device) const;

private:
    void init_physical_device();
    void init_logical_device();
    void init_window();
    void init_surface();
    void init_swapchain();
    void init_image_views();

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    [[nodiscard]] VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    uint32_t rate_device_suitability(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);

    const std::vector<const char*> validationLayers;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
    };
};



#endif //DEVICEHELPER_H
