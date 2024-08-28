
#ifndef DEBUG_H
#define DEBUG_H

#include "../vkinit.h"

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

class InstanceHelper {
public:
    InstanceHelper();
    ~InstanceHelper();

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    static void destroy_debug_utils_messenger_ext(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

private:
    void init_instance();

    bool check_validation_layer_support();
    std::vector<const char*> get_required_extensions();

    static auto debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) -> VkBool32;

    void populate_debug_messenger_CI(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    static VkResult create_debug_utils_messenger_ext(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    void init_debug_messenger();

public:
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
    };
};



#endif //DEBUG_H
