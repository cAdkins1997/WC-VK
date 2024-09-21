
#include "device.hpp"

namespace wcvk::core {
    Buffer Device::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) const {
        VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;

        const auto bufferUsage = usage;
        bufferInfo.usage = bufferUsage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        Buffer newBuffer;

        vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);

        return newBuffer;
    }

    Image* Device::create_image(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped) {
        Image newImage{};
        newImage.imageFormat = format;
        newImage.imageExtent = size;

        VkImageCreateInfo imageCI = vkinit::image_create_info(static_cast<VkFormat>(format) ,static_cast<VkImageUsageFlags>(usage), VkExtent3D(size));
        if (mipmapped) {
            imageCI.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
        }

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(allocator, &imageCI, &allocInfo, &newImage.image, &newImage.allocation, nullptr);

        vk::ImageAspectFlags aspectFlag = vk::ImageAspectFlagBits::eColor;
        if (format  == vk::Format::eD32Sfloat)
            aspectFlag = vk::ImageAspectFlagBits::eDepth;

        vk::ComponentMapping components {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        };

        vk::ImageViewCreateInfo viewInfo({}, newImage.image, vk::ImageViewType::e2D, format, components, aspectFlag);
        viewInfo.subresourceRange.levelCount = imageCI.mipLevels;

        newImage.imageView = device.createImageView(viewInfo, nullptr);

        vk::SamplerCreateInfo samplerCI({}, vk::Filter::eNearest);
        newImage.sampler = device.createSampler(samplerCI, nullptr);

        images.push_back(newImage);
        return images.end();
    }

    Shader Device::create_shader(const char* filePath) const {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        file.is_open() && "failed to open file\n";

        size_t fileSize = static_cast<size_t>(file.tellg());

        eastl::vector<uint32_t> buffer (fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo shaderCI;
        shaderCI.codeSize = buffer.size() * sizeof(uint32_t);
        shaderCI.pCode = buffer.data();

        vk::ShaderModule shaderModule;
        device.createShaderModule(&shaderCI, nullptr, &shaderModule) == vk::Result::eSuccess && "Failed to create shader module";
        const Shader shader { shaderModule };
        return shader;
    }

    void Device::submit_graphics_work(const commands::GraphicsContext &graphicsContext, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal) {
        vk::CommandBuffer cmd = graphicsContext._commandBuffer ;
        vk::CommandBufferSubmitInfo commandBufferSI(cmd);

        FrameData& currentFrame = get_current_frame();
        vk::SemaphoreSubmitInfo waitInfo(currentFrame.swapchainSemaphore);
        waitInfo.stageMask = wait;
        vk::SemaphoreSubmitInfo signalInfo(currentFrame.renderSemaphore);
        signalInfo.stageMask = signal;
        vk::SubmitFlagBits submitFlags{};
        vk::SubmitInfo2 submitInfo(
            submitFlags,
            1, &waitInfo,
            1, &commandBufferSI,
            1, &signalInfo);
        graphicsQueue.submit2( 1, &submitInfo, currentFrame.renderFence);
    }

    void Device::submit_compute_work(const commands::ComputeContext &context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal) {
        vk::CommandBuffer cmd = context._commandBuffer ;
        vk::CommandBufferSubmitInfo commandBufferSI(cmd);

        FrameData& currentFrame = get_current_frame();
        vk::SemaphoreSubmitInfo waitInfo(currentFrame.computeSemaphore);
        waitInfo.stageMask = wait;
        vk::SemaphoreSubmitInfo signalInfo(currentFrame.renderSemaphore);
        signalInfo.stageMask = signal;
        vk::SubmitFlagBits submitFlags{};
        vk::SubmitInfo2 submitInfo(
            submitFlags,
            1, &waitInfo,
            1, &commandBufferSI,
            1, &signalInfo);
        graphicsQueue.submit2( 1, &submitInfo, currentFrame.renderFence);
    }

    void Device::submit_upload_work(const commands::UploadContext &context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal) {
        vk::CommandBuffer cmd = context._commandBuffer ;
        vk::CommandBufferSubmitInfo commandBufferSI(cmd);

        FrameData& currentFrame = get_current_frame();
        vk::SemaphoreSubmitInfo waitInfo(currentFrame.renderSemaphore);
        waitInfo.stageMask = wait;
        vk::SemaphoreSubmitInfo signalInfo(currentFrame.swapchainSemaphore);
        signalInfo.stageMask = signal;
        vk::SubmitFlagBits submitFlags{};
        vk::SubmitInfo2 submitInfo(
            submitFlags,
            1, &waitInfo,
            1, &commandBufferSI,
            1, &signalInfo);
        graphicsQueue.submit2( 1, &submitInfo, currentFrame.renderFence);
    }

    void Device::wait_on_work() {
        device.waitForFences(1, &get_current_frame().renderFence, true, 1000000000);
    }

    void Device::reset_fences() {
        device.resetFences(1, &get_current_frame().renderFence);
    }

    void Device::present() {
        FrameData& currentFrame = get_current_frame();
        vk::PresentInfoKHR presentInfo(1, &currentFrame.renderSemaphore, 1, &swapchain, &swapchainImageIndex);
        auto result = graphicsQueue.presentKHR(&presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR) {
            resizeRequested = true;
            return;
        }
        frameNumber++;
    }

    VkImage& Device::get_swapchain_image() {
        return swapchainImages[get_swapchain_image_index()];
    }

    VkImage& Device::get_draw_image() {
        return drawImage.image;
    }

    Device::Device() {

        if constexpr (wcvk::core::enableValidationLayers == true) {
            std::cout << "\nvalidation layers enabled\n";
        }

        glfwInit();
        if (glfwVulkanSupported()) {
            uint32_t count;
            const char** extensions = glfwGetRequiredInstanceExtensions(&count);

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(width, height, "Window Title", nullptr, nullptr);

            vkb::InstanceBuilder builder;
            auto instantRet = builder.set_app_name("WCVK")
            .request_validation_layers(enableValidationLayers)
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger()
            .build();

            vkb::Instance vkbInstance = instantRet.value();
            instance = vkbInstance.instance;
            debugMessenger = vkbInstance.debug_messenger;

            VkSurfaceKHR tempSurface;
            glfwCreateWindowSurface(instance, window, nullptr, &tempSurface);

            vk::PhysicalDeviceVulkan12Features features12;
            features12.bufferDeviceAddress = true;
            features12.descriptorIndexing = true;
            features12.runtimeDescriptorArray = true;
            features12.drawIndirectCount = true;
            features12.descriptorBindingPartiallyBound = true;
            features12.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
            features12.descriptorBindingStorageImageUpdateAfterBind = true;
            features12.descriptorBindingVariableDescriptorCount = true;

            vk::PhysicalDeviceVulkan13Features features13;
            features13.dynamicRendering = true;
            features13.synchronization2 = true;

            vkb::PhysicalDeviceSelector physicalDeviceSelector{ vkbInstance };
            auto physical_device_selector_ret =
                physicalDeviceSelector.set_surface(tempSurface).
                set_minimum_version(1, 3).
                require_dedicated_transfer_queue().
                set_required_features_12(features12).
                set_required_features_13(features13).
                select();

            surface = vk::SurfaceKHR(tempSurface);

            vkb::DeviceBuilder deviceBuilder { physical_device_selector_ret.value() };
            auto deviceRet = deviceBuilder.build();
            vkb::Device vkbDevice = deviceRet.value();

            physicalDevice = vkbDevice.physical_device;
            device = vkbDevice.device;

            graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
            presentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();
            computeQueue = vkbDevice.get_queue(vkb::QueueType::compute).value();
            transferQueue = vkbDevice.get_queue(vkb::QueueType::transfer).value();

            graphicsQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
            presentQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value();
            computeQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::compute).value();
            transferQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();

            vkb::SwapchainBuilder swapchainBuilder { physicalDevice, vkbDevice, tempSurface };
            swapchainFormat = vk::Format::eR8G8B8A8Unorm;
            auto swapchainRet = swapchainBuilder.build();

            vkb::Swapchain vkbSwapchain = swapchainBuilder.
            set_desired_format(vk::SurfaceFormatKHR(swapchainFormat, vk::ColorSpaceKHR::eSrgbNonlinear)).
            set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).
            set_desired_extent(width, height).
            add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT).
            build().
            value();

            swapchainExtent = vkbSwapchain.extent;
            swapchain = vkbSwapchain.swapchain;

            std::vector<VkImage> imageTransferVector = vkbSwapchain.get_images().value();
            for (auto i : imageTransferVector) {
                swapchainImages.push_back(i);
            }

            std::vector<VkImageView_T*> imageViewTransferVector = vkbSwapchain.get_image_views().value();
            for (auto i : imageViewTransferVector) {
                swapchainImageViews.push_back(i);
            }

            init_commands();
            init_sync_objects();
            init_allocator();
            init_draw_images();
            init_descriptors();
        }
    }

    Device::~Device() {

        device.waitIdle();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            FrameData& frame = frames[i];
            device.destroyCommandPool(frame.commandPool);
            device.destroyFence(frame.renderFence);
            device.destroySemaphore(frame.renderSemaphore);
            device.destroySemaphore(frame.swapchainSemaphore);
            device.destroySemaphore(frame.computeSemaphore);
        }

        vmaDestroyImage(allocator, drawImage.image, nullptr);
        device.destroyImageView(drawImage.imageView, nullptr);
        device.destroySwapchainKHR(swapchain, nullptr);
        instance.destroy(surface);

        vmaDestroyAllocator(allocator);
        device.destroy();

        glfwDestroyWindow(window);
        instance.destroy();

        glfwTerminate();
    }

    void Device::init_commands() {
        vk::CommandPoolCreateInfo commandPoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frames[i].commandPool = device.createCommandPool(commandPoolCI, nullptr);
            vk::CommandBufferAllocateInfo commandBufferAI(frames[i].commandPool, vk::CommandBufferLevel::ePrimary, 1);
            device.allocateCommandBuffers(&commandBufferAI, &frames[i].graphicsCommandBuffer);
            device.allocateCommandBuffers(&commandBufferAI, &frames[i].computeCommandBuffer);
            device.allocateCommandBuffers(&commandBufferAI, &frames[i].uploadCommandBuffer);
        }
    }

    void Device::init_sync_objects() {
        vk::FenceCreateInfo fenceCI(vk::FenceCreateFlagBits::eSignaled);
        vk::SemaphoreCreateInfo semaphoreCI;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device.createFence(&fenceCI, nullptr, &frames[i].renderFence);
            device.createSemaphore(&semaphoreCI, nullptr, &frames[i].swapchainSemaphore);
            device.createSemaphore(&semaphoreCI, nullptr, &frames[i].renderSemaphore);
            device.createSemaphore(&semaphoreCI, nullptr, &frames[i].computeSemaphore);
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
    }

    void Device::init_draw_images() {
        VkExtent3D drawImageExtent = { width, height, 1 };

        drawImage.imageFormat = static_cast<vk::Format>(VK_FORMAT_R16G16B16A16_SFLOAT);
        drawImage.imageExtent = drawImageExtent;

         VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkImageCreateInfo rimg_info = vkinit::image_create_info(static_cast<VkFormat>(drawImage.imageFormat), drawImageUsages, drawImageExtent);

        VmaAllocationCreateInfo rimg_allocinfo = {};
        rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimg_allocinfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &drawImage.image, &drawImage.allocation, nullptr);

        VkComponentMapping components {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };

        auto subresourceRange = vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
        VkImageViewCreateInfo viewInfo = vkinit::image_view_CI(drawImage.image, VK_IMAGE_VIEW_TYPE_2D, static_cast<VkFormat>(drawImage.imageFormat), components, subresourceRange);

        vkCreateImageView(device, &viewInfo, nullptr, reinterpret_cast<VkImageView*>(&drawImage.imageView));
    }
}
