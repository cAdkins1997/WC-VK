
#include "vkinit.h"

VkApplicationInfo vkinit::app_info(const char *appName, uint32_t appVersion, uint32_t engineVersion, uint32_t apiVersion) {
        VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = appName;
        appInfo.applicationVersion = apiVersion;
        appInfo.apiVersion = apiVersion;
        appInfo.engineVersion = engineVersion;
        appInfo.apiVersion = apiVersion;
        return appInfo;
}

VkDeviceQueueCreateInfo vkinit::queue_CI(uint32_t familyIndex, uint32_t queueCount) {
    VkDeviceQueueCreateInfo queueCreateInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = familyIndex;
    queueCreateInfo.queueCount = queueCount;
    return queueCreateInfo;
}

VkSwapchainCreateInfoKHR vkinit::swapchain_CI(VkSurfaceKHR surface,
    uint32_t minImageCount,
    VkSurfaceFormatKHR surfaceFormat,
    VkExtent2D extent,
    uint32_t imageArrayLayers,
    VkImageUsageFlags imageUsage)
{
    VkSwapchainCreateInfoKHR swapchainCI{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCI.surface = surface;
    swapchainCI.minImageCount = minImageCount;
    swapchainCI.imageFormat = surfaceFormat.format;
    swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCI.imageExtent = extent;
    swapchainCI.imageArrayLayers = imageArrayLayers;
    swapchainCI.imageUsage = imageUsage;
    return swapchainCI;
}

VkImageViewCreateInfo vkinit::image_view_CI(
    VkImage image,
    VkImageViewType imageType,
    VkFormat format,
    VkComponentMapping componentMapping,
    const VkImageSubresourceRange &subresourceRange
    )
{
    VkImageViewCreateInfo imageViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    imageViewCI.image = image;
    imageViewCI.viewType = imageType;
    imageViewCI.format = format;
    imageViewCI.components = componentMapping;
    imageViewCI.subresourceRange = subresourceRange;
    return imageViewCI;
}

VkImageCreateInfo vkinit::image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
    VkImageCreateInfo info = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    info.pNext = nullptr;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.extent = extent;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;
    return info;
}

VkImageSubresourceRange vkinit::subresource_range(
    VkImageAspectFlags imageAspectFlags,
    uint32_t baseMipLevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount)
{
    return { imageAspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};
}

VkDescriptorSetLayoutBindingFlagsCreateInfo vkinit::ds_layout_binding_flags_CI(
    const VkDescriptorBindingFlags *bindingFlags, uint32_t bindingCount)
{
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCI{};
    bindingFlagsCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCI.pNext = nullptr;
    bindingFlagsCI.pBindingFlags = bindingFlags;
    bindingFlagsCI.bindingCount = bindingCount;
    return bindingFlagsCI;
}

VkDescriptorSetLayoutCreateInfo vkinit::ds_layout_CI(uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags) {
    VkDescriptorSetLayoutCreateInfo dsLayoutCI{};
    dsLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsLayoutCI.bindingCount = bindingCount;
    dsLayoutCI.flags = flags;
    return dsLayoutCI;
}

VkDescriptorPoolCreateInfo vkinit::ds_pool_CI(
    VkDescriptorPoolCreateFlags flags,
    uint32_t maxSets,
    uint32_t poolSizeCount,
    const VkDescriptorPoolSize *pPoolSizes)
{
    VkDescriptorPoolCreateInfo dsPoolCI{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    dsPoolCI.pNext = nullptr;
    dsPoolCI.flags = flags;
    dsPoolCI.maxSets = maxSets;
    dsPoolCI.poolSizeCount = poolSizeCount;
    dsPoolCI.pPoolSizes = pPoolSizes;
    return dsPoolCI;
}

VkDescriptorSetAllocateInfo vkinit::ds_ai(VkDescriptorPool ds, uint32_t count) {
    VkDescriptorSetAllocateInfo allocateInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = ds;
    allocateInfo.descriptorSetCount = count;
    return allocateInfo;
}
