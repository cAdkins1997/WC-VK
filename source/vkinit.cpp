
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

VkCommandPoolCreateInfo vkinit::command_pool_CI(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo commandPoolCI { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    commandPoolCI.pNext = nullptr;
    commandPoolCI.flags = flags;
    commandPoolCI.queueFamilyIndex = queueFamilyIndex;
    return commandPoolCI;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_AI(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count) {
    VkCommandBufferAllocateInfo commandBufferAI { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    commandBufferAI.pNext = nullptr;
    commandBufferAI.commandPool = commandPool;
    commandBufferAI.level = level;
    commandBufferAI.commandBufferCount = count;
    return commandBufferAI;
}

VkCommandBufferBeginInfo vkinit::command_buffer_BI(VkCommandBufferUsageFlags usageFlags) {
    VkCommandBufferBeginInfo commandBufferBI { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    commandBufferBI.flags = usageFlags;
    commandBufferBI.pNext = nullptr;
    return commandBufferBI;
}

VkCommandBufferSubmitInfo vkinit::command_buffer_SI(VkCommandBuffer commandBuffer) {
    VkCommandBufferSubmitInfo submitInfo { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .pNext = nullptr };
    submitInfo.commandBuffer = commandBuffer;
    submitInfo.deviceMask = 0;
    return submitInfo;
}

VkSubmitInfo2 vkinit::submit_info(
    VkCommandBufferSubmitInfo *cmd,
    VkSemaphoreSubmitInfo *signalSemaphoreInfo,
    VkSemaphoreSubmitInfo *waitSemaphoreInfo)
{
    VkSubmitInfo2 submitInfo { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .pNext =nullptr };
    submitInfo.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfo;

    submitInfo.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfo;

    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = cmd;

    return submitInfo;
}

VkSemaphoreSubmitInfo vkinit::semaphore_SI(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
    VkSemaphoreSubmitInfo submitInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .pNext = nullptr };
    submitInfo.semaphore = semaphore;
    submitInfo.stageMask = stageMask;
    submitInfo.deviceIndex = 0;
    submitInfo.value = 1;
    return submitInfo;
}

VkPresentInfoKHR vkinit::present_info(uint32_t swapchainCount, uint32_t semaphoreCount) {
    VkPresentInfoKHR presentInfo { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr };
    presentInfo.swapchainCount = swapchainCount;
    presentInfo.waitSemaphoreCount = semaphoreCount;
    return presentInfo;
}

VkRenderingInfo vkinit::rendering_info(
    VkRenderingFlags flags,
    VkRect2D area,
    uint32_t layerCount,
    uint32_t viewMask,
    uint32_t colorAttachmentCount)
{
    VkRenderingInfo renderingInfo { .sType =  VK_STRUCTURE_TYPE_RENDERING_INFO, .pNext = nullptr };
    renderingInfo.flags = flags;
    renderingInfo.renderArea = area;
    renderingInfo.layerCount = layerCount;
    renderingInfo.viewMask = viewMask;
    renderingInfo.colorAttachmentCount = colorAttachmentCount;
}

VkRenderingAttachmentInfo vkinit::rendering_attachment_info(
    VkImageView view,
    VkImageLayout layout,
    VkResolveModeFlagBits resolveMode,
    VkImageView resolveView,
    VkImageLayout resolveLayout,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkClearValue clearValue)
{
    VkRenderingAttachmentInfo renderingAttachmentInfo { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, .pNext = nullptr };
    renderingAttachmentInfo.imageView = view;
    renderingAttachmentInfo.imageLayout = layout;
    renderingAttachmentInfo.resolveMode = resolveMode;
    renderingAttachmentInfo.resolveImageView = resolveView;
    renderingAttachmentInfo.resolveImageLayout = resolveLayout;
    renderingAttachmentInfo.loadOp = loadOp;
    renderingAttachmentInfo.storeOp = storeOp;
    renderingAttachmentInfo.clearValue = clearValue;
    return renderingAttachmentInfo;
}

VkFenceCreateInfo vkinit::fence_CI(VkFenceCreateFlags flags) {
    VkFenceCreateInfo fenceCI { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext =nullptr };
    fenceCI.flags = flags;
    return fenceCI;
}

VkSemaphoreCreateInfo vkinit::semaphore_CI(VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo semaphore_CI { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext =nullptr };
    semaphore_CI.flags = flags;
    return semaphore_CI;
}

VkDescriptorImageInfo vkinit::ds_image_info(VkImageLayout layout, VkImageView imageView, VkSampler sampler) {
    VkDescriptorImageInfo imageInfo {};
    imageInfo.imageLayout = layout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    return imageInfo;
}

VkWriteDescriptorSet vkinit::write_ds(
    VkDescriptorType type,
    uint32_t dstBinding,
    VkDescriptorSet dstSet,
    uint32_t count,
    uint32_t dstArrayElement)
{
    VkWriteDescriptorSet writeSet{};
    writeSet.descriptorType = type;
    writeSet.dstBinding = dstBinding;
    writeSet.dstSet = dstSet;
    writeSet.descriptorCount = count;
    writeSet.dstArrayElement = dstArrayElement;
    return writeSet;
}

VkDescriptorBufferInfo vkinit::ds_buffer_info(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;
    return bufferInfo;
}
