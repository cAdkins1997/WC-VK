
#include "device.h"

Device::Device() {
    init_window();
    init_instance();
    init_debug_messenger();
    init_surface();
    init_physical_device();
    init_logical_device();
    init_swapchain();
    init_commands();
    init_sync_objects();
    init_image_views();
    init_allocator();
    init_descriptors();
}

Device::~Device() {
    vkDeviceWaitIdle(device);

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    for (auto buffer : buffers) {
        vkDestroyBuffer(device, buffer, nullptr);
    }

    for (auto image : images) {
        vmaDestroyImage(allocator, image.image, image.allocation);
    }

    for (auto shader : shaders) {
        vkDestroyShaderModule(device, shader.module, nullptr);
    }

    for (auto & frame : frames) {
        vkDestroyCommandPool(device, frame.commandPool, nullptr);
        vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);
        vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
        vkDestroyFence(device, frame.renderFence, nullptr);
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwTerminate();
    if (enableValidationLayers) {
        destroy_debug_utils_messenger_ext(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
}

BufferHandle Device::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
}

TextureHandle Device::create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
    Image image{};
    image.imageFormat = format;
    image.imageExtent = size;

    VkImageCreateInfo imageCI = vkinit::image_create_info(format, usage, size);
    if (mipmapped) {
        imageCI.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    }

    VmaAllocationCreateInfo vmaAI{};
    vmaAI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaAI.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vmaCreateImage(allocator, &imageCI, &vmaAI, &image.image, &image.allocation, nullptr));

    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    VkComponentMapping components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
    };

    VkImageView imageView{};
    VkImageSubresourceRange subresourceRange = vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    VkImageViewCreateInfo imageViewInfo = vkinit::image_view_CI(image.image, VK_IMAGE_VIEW_TYPE_2D, format, components, subresourceRange);
    vkCreateImageView(device, &imageViewInfo, nullptr, &imageView);

    VkSampler sampler;
    vkinit::SamplerAddressModes addressModes {
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
    VkSamplerCreateInfo sampler_CI =
        vkinit::sampler_CI(VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT,
            VK_FILTER_NEAREST,
            VK_FILTER_NEAREST,
            addressModes,
            VK_SAMPLER_MIPMAP_MODE_NEAREST);
    vkCreateSampler(device, &sampler_CI, nullptr, &sampler);

    return store_texture(imageView, sampler);
}

VShader Device::create_shader(const char *filePath) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    size_t fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    assert(file.is_open());

    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCI{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    shaderModuleCI.pNext = nullptr;
    shaderModuleCI.codeSize = buffer.size() * sizeof(uint32_t);
    shaderModuleCI.pCode = buffer.data();

    VkShaderModule shaderModule;

    VK_CHECK(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));
    Shader shader { shaderModule };
    shaders.push_back(shader);
    return &shaders[shaders.size()];
}

VPipeline Device::create_pipeline(VkPipelineCreateFlagBits type) {

    Pipeline pipeline{};
    pipelines.push_back(pipeline);
    return &pipelines[pipelines.size()];
}

void Device::submit_graphics_work(GraphicsContext &context) {
    VkCommandBuffer cmd = context.commandBuffer;
    VkCommandBufferSubmitInfo commandBufferSI = vkinit::command_buffer_SI(cmd);

    FrameData& currentFrame = get_current_frame();
    VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_SI(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_SI(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, currentFrame.renderSemaphore);
    VkSubmitInfo2 submitInfo = vkinit::submit_info(&commandBufferSI, &signalInfo, &waitInfo);
    vkQueueSubmit2(graphicsQueue, 1, &submitInfo, currentFrame.renderFence);
}

void Device::submit_compute_work(ComputeContext &context) {

}

void Device::submit_raytracing_work(RaytracingContext &context) {

}

void Device::submit_upload_work(UploadContext &context) {

}

void Device::wait_on_work() {
    vkWaitForFences(device, 1, &get_current_frame().renderFence, true, 1000000000);
    vkResetFences(device, 1, &get_current_frame().renderFence);
}

void Device::present() {
    FrameData& currentFrame = get_current_frame();
    VkPresentInfoKHR presentInfo = vkinit::present_info(1, 1);
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.pImageIndices = &swapchainImageIndex;
    vkQueuePresentKHR(graphicsQueue, &presentInfo);
    frameNumber++;
}

TextureHandle Device::store_texture(VkImageView imageView, VkSampler sampler) {
    size_t handle = texturesToUpdate.size();
    textures.push_back(imageView);

    VkDescriptorImageInfo imageInfo = vkinit::ds_image_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageView, sampler);

    VkWriteDescriptorSet write = vkinit::write_ds(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TextureBinding, descriptorSet, 1, handle);
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    return static_cast<TextureHandle>(handle);
}

BufferHandle Device::store_buffer(VkBuffer buffer, VkBufferUsageFlagBits usage) {
    size_t handle = buffers.size();
    buffers.push_back(buffer);

    std::array<VkWriteDescriptorSet, 2> writes{};
    for (auto& write : writes) {
        VkDescriptorBufferInfo bufferInfo = vkinit::ds_buffer_info(buffer, 0, VK_WHOLE_SIZE);
        write = vkinit::write_ds(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, descriptorSet, 1, handle);
    }

    size_t index = 0;
    if ((usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        writes.at(index).dstBinding = UniformBinding;
        writes.at(index).descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }

    if ((usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        writes.at(index).dstBinding = StorageBinding;
        writes.at(index).descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }

    vkUpdateDescriptorSets(device, index, writes.data(), 0, nullptr);

    return static_cast<BufferHandle>(handle);
}

void Device::init_window() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "WCVK", nullptr, nullptr);
}

void Device::init_instance() {
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

    VK_CHECK(vkCreateInstance(&instanceCI, nullptr, &instance));
}

void Device::init_debug_messenger() {
    if constexpr (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugCI{};
    populate_debug_messenger_CI(debugCI);

    VK_CHECK(create_debug_utils_messenger_ext(instance, &debugCI, nullptr, &debugMessenger));
}

void Device::init_surface() {
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}

void Device::init_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<uint32_t, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        uint32_t score = rate_device_suitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
    }
}

void Device::init_logical_device() {
    QueueFamilyIndices indices = find_queue_families(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceBufferDeviceAddressFeatures deviceBufferAddressFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
    deviceBufferAddressFeatures.bufferDeviceAddress = true;

    VkPhysicalDeviceDescriptorIndexingFeatures descIndexingFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
    descIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
    descIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
    descIndexingFeatures.descriptorBindingPartiallyBound = true;
    descIndexingFeatures.runtimeDescriptorArray = true;
    descIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
    descIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
    descIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
    descIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
    descIndexingFeatures.pNext = &deviceBufferAddressFeatures;

    VkPhysicalDeviceSynchronization2Features sync2Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
    sync2Features.pNext = &descIndexingFeatures;


    VkPhysicalDeviceFeatures2 deviceFeatures2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    deviceFeatures2.pNext = &sync2Features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    VkDeviceCreateInfo deviceCI { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCI.pQueueCreateInfos = queueCreateInfos.data();
    deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCI.pEnabledFeatures = nullptr;
    deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCI.pNext = &deviceFeatures2;\

    if (enableValidationLayers) {
        deviceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCI.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCI.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    graphicsQueueIndex = indices.graphicsFamily.value();
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
    computeQueueIndex = indices.computeFamily.value();
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    presentQueueIndex = indices.presentFamily.value();
}

void Device::init_swapchain() {
    SwapChainSupportDetails swapChainSupport = query_swap_chain_support(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
    VkExtent2D extent = choose_swap_extent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCI =
        vkinit::swapchain_CI(surface, imageCount, surfaceFormat, extent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    QueueFamilyIndices indices = find_queue_families(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCI.queueFamilyIndexCount = 2;
        swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.queueFamilyIndexCount = 0;
        swapchainCI.pQueueFamilyIndices = nullptr;
    }
    swapchainCI.preTransform = swapChainSupport.capabilities.currentTransform;
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = presentMode;
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
    swapchainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Device::init_commands() {
    VkCommandPoolCreateInfo commandPoolCI =
        vkinit::command_pool_CI(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueIndex);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateCommandPool(device, &commandPoolCI, nullptr, &frames[i].commandPool);
        VkCommandBufferAllocateInfo commandBufferAI = vkinit::command_buffer_AI(frames[i].commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        vkAllocateCommandBuffers(device, &commandBufferAI, &frames[i].commandBuffer);
    }
}

void Device::init_sync_objects() {
    VkFenceCreateInfo fenceCI = vkinit::fence_CI(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCI = vkinit::semaphore_CI(0);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateFence(device, &fenceCI, nullptr, &frames[i].renderFence);
        vkCreateSemaphore(device, &semaphoreCI, nullptr, &frames[i].swapchainSemaphore);
        vkCreateSemaphore(device, &semaphoreCI, nullptr, &frames[i].renderSemaphore);
    }
}

void Device::init_image_views() {
    swapChainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {

        VkComponentMapping components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };

        VkImageSubresourceRange subresourceRange = vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        VkImageViewCreateInfo imageViewCI = vkinit::image_view_CI(swapchainImages[i], VK_IMAGE_VIEW_TYPE_2D, swapchainImageFormat, components, subresourceRange);

        VK_CHECK(vkCreateImageView(device, &imageViewCI, nullptr, &swapChainImageViews[i]));
    }
}

void Device::init_allocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Device::init_descriptors() {
    descriptors::DescriptorSetBuilder builder;
    builder.build(device, descriptorSetLayout, descriptorPool, descriptorSet);
}

bool Device::check_validation_layer_support() {
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

std::vector<const char *> Device::get_required_extensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

auto Device::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) -> VkBool32 {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Device::populate_debug_messenger_CI(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_callback;
}

VkResult Device::create_debug_utils_messenger_ext(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT*pDebugMessenger) {
    if (auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Device::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
}

uint32_t Device::rate_device_suitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    VkPhysicalDeviceVulkan12Features features12 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    VkPhysicalDeviceVulkan13Features features13 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.pNext = &features12;

    VkPhysicalDeviceFeatures2 features2 { .sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext =  &features13 };
    vkGetPhysicalDeviceFeatures2(device, &features2);

    uint32_t score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    QueueFamilyIndices indices = find_queue_families(device);
    if (indices.graphicsFamily.has_value() && indices.presentFamily.has_value() && indices.computeFamily.has_value()) {
        score += 1000;
    }

    if (check_device_extension_support(device)) {
        SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device);
        if (!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty()) {
            score += 10000;
        }
    }

    return score;
}

Device::QueueFamilyIndices Device::find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices{};


    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool Device::check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Device::SwapChainSupportDetails Device::query_swap_chain_support(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Device::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
}

VkPresentModeKHR Device::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Device::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}


