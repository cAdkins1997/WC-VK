
#ifndef VKINIT_H
#define VKINIT_H

#include "vkcommon.h"

namespace vkinit {

    VkApplicationInfo app_info(const char* appName, uint32_t appVersion, uint32_t engineVersion, uint32_t apiVersion);

    VkDeviceQueueCreateInfo queue_CI(uint32_t familyIndex, uint32_t queueCount);

    VkSwapchainCreateInfoKHR swapchain_CI(
        VkSurfaceKHR surface,
        uint32_t minImageCount,
        VkSurfaceFormatKHR surfaceFormat,
        VkExtent2D extent,
        uint32_t imageArrayLayers,
        VkImageUsageFlags imageUsage
        );

    VkImageViewCreateInfo image_view_CI(
        VkImage image,
        VkImageViewType imageType,
        VkFormat format,
        VkComponentMapping componentMapping,
        const VkImageSubresourceRange &subresourceRange
        );

    VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);


    VkImageSubresourceRange subresource_range(
        VkImageAspectFlags imageAspectFlags,
        uint32_t baseMipLevel,
        uint32_t levelCount,
        uint32_t baseArrayLayer,
        uint32_t layerCount
        );

    VkDescriptorSetLayoutBindingFlagsCreateInfo ds_layout_binding_flags_CI(
        const VkDescriptorBindingFlags *bindingFlags, uint32_t bindingCount);

    VkDescriptorSetLayoutCreateInfo ds_layout_CI(
        uint32_t bindingCount,
        VkDescriptorSetLayoutCreateFlags flags);

    VkDescriptorPoolCreateInfo ds_pool_CI(
        VkDescriptorPoolCreateFlags flags,
        uint32_t maxSets,
        uint32_t poolSizeCount,
        const VkDescriptorPoolSize* pPoolSizes
        );

    VkDescriptorSetAllocateInfo ds_ai(VkDescriptorPool ds, uint32_t count);

    VkCommandPoolCreateInfo command_pool_CI(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    VkCommandBufferAllocateInfo command_buffer_AI(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count);
    VkCommandBufferBeginInfo command_buffer_BI(VkCommandBufferUsageFlags usageFlags);
    VkCommandBufferSubmitInfo command_buffer_SI(VkCommandBuffer commandBuffer);
    VkSubmitInfo2 submit_info(
        VkCommandBufferSubmitInfo* cmd,
        VkSemaphoreSubmitInfo* signalSemaphoreInfo,
        VkSemaphoreSubmitInfo* waitSemaphoreInfo);

    VkSemaphoreSubmitInfo semaphore_SI(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);

    VkPresentInfoKHR present_info(uint32_t swapchainCount, uint32_t semaphoreCount);

    VkRenderingInfo rendering_info(VkRenderingFlags flags, VkRect2D area, uint32_t layerCount, uint32_t viewMask, uint32_t colorAttachmentCount);

    VkRenderingAttachmentInfo rendering_attachment_info(
        VkImageView view,
        VkImageLayout layout,
        VkResolveModeFlagBits resolveMode,
        VkImageView resolveView,
        VkImageLayout resolveLayout,
        VkAttachmentLoadOp loadOp,
        VkAttachmentStoreOp storeOp,
        VkClearValue clearValue
        );

    VkFenceCreateInfo fence_CI(VkFenceCreateFlags flags);

    VkSemaphoreCreateInfo semaphore_CI(VkSemaphoreCreateFlags flags);
}

#endif //VKINIT_H
