
#include "InstanceHelper.h"

InstanceHelper::InstanceHelper() {
    glfwInit();
    init_instance();
    init_debug_messenger();
}

InstanceHelper::~InstanceHelper() {

}

void InstanceHelper::init_instance() {
    VkApplicationInfo appInfo = vkinit::app_info("WCVK", 0, 0, VK_MAKE_VERSION(1, 3, 0));
    VkInstanceCreateInfo instanceCI { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCI.pApplicationInfo = &appInfo;

    auto extensions = get_required_extensions();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCI.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCI{};
    VkValidationFeaturesEXT validationFeatures{ .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
    if constexpr (enableValidationLayers) {
        validationFeatures.enabledValidationFeatureCount = enabledValidationFeatures.size();
        validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.data();
        validationFeatures.pNext = &debugCI;
        instanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCI.ppEnabledLayerNames = validationLayers.data();
        populate_debug_messenger_CI(debugCI);
        instanceCI.pNext = &validationFeatures;
    } else {
        instanceCI.enabledLayerCount = 0;
        instanceCI.pNext = nullptr;
    }

    vkCreateInstance(&instanceCI, nullptr, &instance);
}

bool InstanceHelper::check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> InstanceHelper::get_required_extensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

auto InstanceHelper::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) -> VkBool32 {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void InstanceHelper::populate_debug_messenger_CI(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_callback;
}

VkResult InstanceHelper::create_debug_utils_messenger_ext(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT*pDebugMessenger) {
    if (auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void InstanceHelper::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void InstanceHelper::init_debug_messenger() {
    if constexpr (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugCI{};
    populate_debug_messenger_CI(debugCI);

    create_debug_utils_messenger_ext(instance, &debugCI, nullptr, &debugMessenger);
}
