
#include "device.h"

Device::Device() : deviceHelper(instanceHelper) {
    device = deviceHelper.device;
    physicalDevice = deviceHelper.physicalDevice;
    surface = deviceHelper.surface;
    swapchain = deviceHelper.swapchain;
    swapchainImages = deviceHelper.swapchainImages;
    swapChainImageViews = deviceHelper.swapChainImageViews;
    window = deviceHelper.window;
    graphicsQueue = deviceHelper.graphicsQueue;
    computeQueue = deviceHelper.computeQueue;
    presentQueue = deviceHelper.presentQueue;
    deviceHelper.assign_queue_family_indices(graphicsQueueIndex, computeQueueIndex, presentQueueIndex);

    init_commands();
    init_sync_objects();
    init_allocator();
    init_descriptors();
}

Device::~Device() {
    vkDeviceWaitIdle(device);

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);

    for (auto & frame : frames) {
        vkDestroyCommandPool(device, frame.commandPool, nullptr);
        vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);
        vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
        vkDestroyFence(device, frame.renderFence, nullptr);
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    if (enableValidationLayers) {
        InstanceHelper::destroy_debug_utils_messenger_ext(instance, instanceHelper.debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}

TextureHandle Device::create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
    Image image{};
    image.imageFormat = format;
    image.imageExtent = size;

    auto imageCI = vkinit::image_create_info(format, usage, size);
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
    auto subresourceRange = vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    auto imageViewInfo = vkinit::image_view_CI(image.image, VK_IMAGE_VIEW_TYPE_2D, format, components, subresourceRange);
    vkCreateImageView(device, &imageViewInfo, nullptr, &imageView);
    image.imageView = imageView;

    VkSampler sampler;
    vkinit::SamplerAddressModes addressModes {
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
    auto sampler_CI = vkinit::sampler_CI(
        0,
        VK_FILTER_NEAREST,
        VK_FILTER_NEAREST,
        addressModes,
        VK_SAMPLER_MIPMAP_MODE_NEAREST);
    vkCreateSampler(device, &sampler_CI, nullptr, &sampler);
    image.sampler = sampler;

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
}

void Device::submit_graphics_work(GraphicsContext &context) {
    auto cmd = context.commandBuffer;
    auto commandBufferSI = vkinit::command_buffer_SI(cmd);

    FrameData& currentFrame = get_current_frame();
    auto waitInfo = vkinit::semaphore_SI(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    auto signalInfo = vkinit::semaphore_SI(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, currentFrame.renderSemaphore);
    auto submitInfo = vkinit::submit_info(&commandBufferSI, &signalInfo, &waitInfo);
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

uint32_t Device::get_min_uniform_buffer_offset_alignment() const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    return static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
}

TextureHandle Device::store_texture(VkImageView imageView, VkSampler sampler) {
    size_t handle = textures.size();
    textures.push_back(imageView);

    auto imageInfo = vkinit::ds_image_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageView, sampler);

    auto writeDS = vkinit::write_ds(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TextureBinding, descriptorSet, 1, handle);
    writeDS.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(device, 1, &writeDS, 0, nullptr);

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

void Device::init_allocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance =  instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &allocator);
}

void Device::init_descriptors() {
    using namespace descriptors;
    size_t minUniformBufferOffsetAlignment = get_min_uniform_buffer_offset_alignment();
    BindlessParams bindlessParams(minUniformBufferOffsetAlignment);

    auto rangePBR = bindlessParams.add_range(PBRParams({meshTransformsBuffer, pointLightsBuffer, cameraBuffer });
    auto rangeSkybox = bindlessParams.add_range(SkyBoxParams({cameraBuffer, skyboxTexture});

    bindlessParams.build(device, allocator, descriptorPool);
    vkCmdBindDescriptorSets()
}


