
#include "device.hpp"

namespace wcvk::core {
    void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
        auto xpos = static_cast<float>(xposIn);
        auto ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.process_mouse_movement(xoffset, yoffset, false);
    }

    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
        camera.process_mouse_scroll(static_cast<float>(yoffset));
    }

    void process_input(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.process_keyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.process_keyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.process_keyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.process_keyboard(RIGHT, deltaTime);
    }

    Buffer Device::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) const {
        VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;

        const auto bufferUsage = usage;
        bufferInfo.usage = bufferUsage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = flags;
        Buffer newBuffer{};

        vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);

        vmaGetAllocationMemoryProperties(allocator, newBuffer.allocation, &newBuffer.memoryProperties);

        return newBuffer;
    }

    Image Device::create_image(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage, bool mipmapped) {
        Image newImage{};
        newImage.imageFormat = format;
        newImage.imageExtent = size;

        VkImageCreateInfo imageCI = vkinit::image_create_info(
            static_cast<VkFormat>(format),
            static_cast<VkImageUsageFlags>(usage),
            VkExtent3D(size));\

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
        viewInfo.subresourceRange.layerCount = vk::RemainingArrayLayers;

        newImage.imageView = device.createImageView(viewInfo, nullptr);

        /*vk::SamplerCreateInfo samplerCI({}, vk::Filter::eNearest);
        newImage.sampler = device.createSampler(samplerCI, nullptr);*/

        return newImage;
    }

    vk::Sampler Device::create_sampler(vk::Filter minFilter, vk::Filter magFilter) {
        vk::SamplerCreateInfo samplerCI;
        samplerCI.minFilter = minFilter;
        samplerCI.magFilter = magFilter;
        vk::Sampler newSampler;
        vk_check(
            device.createSampler(&samplerCI, nullptr, &newSampler),
            "failed to create sampler"
            );
        return newSampler;
    }

    Shader Device::create_shader(std::string_view filePath) const {
        std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);

        if (file.is_open() == false) {
            throw std::runtime_error("Failed to find file!\n");
        }

        size_t fileSize = static_cast<size_t>(file.tellg());

        std::vector<uint32_t> buffer (fileSize / sizeof(uint32_t));

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
        if (auto result = graphicsQueue.submit2( 1, &submitInfo, currentFrame.renderFence); result != vk::Result::eSuccess) {
            auto stringResult = vk::to_string(result);
            std::string string = "Failed to submit graphics commands. Error: ";
            string.append(stringResult);
            throw std::runtime_error(string);
        }
    }

    void Device::submit_compute_work(const commands::ComputeContext &context, vk::PipelineStageFlagBits2 wait, vk::PipelineStageFlagBits2 signal) {
        vk::CommandBuffer cmd = context._commandBuffer ;
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


        if (vk::Result result = graphicsQueue.submit2(1, &submitInfo, currentFrame.renderFence); result != vk::Result::eSuccess) {
            auto stringResult = vk::to_string(result);
            std::string string = "Failed to submit compute commands. Error: ";
            string.append(stringResult);
            throw std::runtime_error(string);
        }
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
        if (auto result = graphicsQueue.submit2(1, &submitInfo, currentFrame.renderFence); result != vk::Result::eSuccess) {
            auto stringResult = vk::to_string(result);
            std::string string = "Failed to submit upload commands. Error: ";
            string.append(stringResult);
            throw std::runtime_error(string);
        }
    }

    void Device::submit_upload_work(const commands::UploadContext &context) {
        vk::CommandBuffer cmd = context._commandBuffer ;
        vk::CommandBufferSubmitInfo commandBufferSI(cmd);
        vk::SubmitInfo2 submitInfo({}, nullptr, commandBufferSI);
        vk_check(
            graphicsQueue.submit2(1, &submitInfo, immediateFence),
            "Failed to submit immediate upload commands"
            );

        vk_check(
            device.waitForFences(1, &immediateFence, true, UINT64_MAX),
            "Failed to wait for fences"
            );
    }

    void Device::submit_immediate_work(std::function<void(VkCommandBuffer cmd)> &&function) {
        vk_check(device.resetFences(1, &immediateFence), "Failed to reset fences");

        immediateCommandBuffer.reset({});

        vk::CommandBufferBeginInfo commandBufferBI(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        vk_check(immediateCommandBuffer.begin(&commandBufferBI), "Failed to beginm command buffer");

        function(immediateCommandBuffer);
        immediateCommandBuffer.end();

        vk::CommandBufferSubmitInfo commandBufferSI(immediateCommandBuffer);
        vk::SubmitInfo2 submitInfo({}, nullptr, nullptr);
        vk_check(
            graphicsQueue.submit2(1, &submitInfo, immediateFence),
            "Failed to submit immediate upload commands"
            );

        vk_check(
            device.waitForFences(1, &immediateFence, true, UINT64_MAX),
            "Failed to wait for fences"
            );
    }

    void Device::wait_on_work() {
        vk::Fence* fences = &get_current_frame().renderFence;
        vk_check(device.waitForFences(1, fences, true, UINT64_MAX), "Failed to wait for fences");
        vk_check(device.resetFences(1, fences), "Failed to reset fences");
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

    vkctx Device::build_vcktx(QueueType queueType, vk::CommandPool commandPool) const {
        vk::Queue queueToUse{};
        switch (queueType) {
            case Graphics:
                queueToUse = graphicsQueue;
                break;
            case Compute:
                queueToUse = computeQueue;
                break;
            case Transfer:
                queueToUse = transferQueue;
                break;
        }

        return {physicalDevice, device, queueToUse, commandPool};
    }

    Device::Device() {

        if constexpr (wcvk::core::enableValidationLayers == true) {
            std::cout << "\nvalidation layers enabled\n";
        }

        glfwInit();
        if (glfwVulkanSupported()) {
            uint32_t count;
            const char** extensions = glfwGetRequiredInstanceExtensions(&count);

            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(width, height, "Window Title", nullptr, nullptr);
            glfwSetCursorPosCallback(window, mouse_callback);
            glfwSetScrollCallback(window, scroll_callback);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwMakeContextCurrent(window);


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
                add_required_extension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME).
                add_required_extension(VK_KHR_SPIRV_1_4_EXTENSION_NAME).
                add_required_extension("VK_EXT_mesh_shader").
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

            swapchainImages = vkbSwapchain.get_images().value();
            swapchainImageViews = vkbSwapchain.get_image_views().value();

            init_commands();
            init_sync_objects();
            init_allocator();
            init_draw_images();
            init_depth_images();
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
        }
        device.destroyCommandPool(immediateCommandPool);
        device.destroyFence(immediateFence);

        primaryDeletionQueue.flush();
    }

    void Device::init_commands() {
        vk::CommandPoolCreateInfo commandPoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frames[i].commandPool = device.createCommandPool(commandPoolCI, nullptr);
            vk::CommandBufferAllocateInfo commandBufferAI(frames[i].commandPool, vk::CommandBufferLevel::ePrimary, 1);
            vk_check(device.allocateCommandBuffers(&commandBufferAI, &frames[i].commandBuffer), "Failed to allocate command buffers");
        }

        {
            vk::CommandPoolCreateInfo immPoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
            immediateCommandPool = device.createCommandPool(immPoolCI, nullptr);

            vk::CommandBufferAllocateInfo commandBufferAI(immediateCommandPool, vk::CommandBufferLevel::ePrimary, 1);
            vk_check(device.allocateCommandBuffers(&commandBufferAI, &immediateCommandBuffer), "Failed to allocate command buffers");
        }
    }

    void Device::init_sync_objects() {
        vk::FenceCreateInfo fenceCI(vk::FenceCreateFlagBits::eSignaled);
        vk::SemaphoreCreateInfo semaphoreCI;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk_check(device.createFence(&fenceCI, nullptr, &frames[i].renderFence), "Failed to create fence");
            vk_check(device.createSemaphore(&semaphoreCI, nullptr, &frames[i].swapchainSemaphore), "Failed to create semaphore");
            vk_check(device.createSemaphore(&semaphoreCI, nullptr, &frames[i].renderSemaphore), "Failed to create semaphore");
        }

        vk_check(device.createFence(&fenceCI, nullptr, &immediateFence), "Failed to create fence");
    }

    void Device::init_allocator() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance =  instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &allocator);

        primaryDeletionQueue.push_function([&]() {
            vmaDestroyAllocator(allocator);
        });
    }

    void Device::init_descriptors() {
    }

    void Device::init_draw_images() {
        VkExtent3D drawImageExtent = { width, height, 1 };

        drawImage.imageFormat = vk::Format::eR16G16B16A16Sfloat;
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

        drawImageSamplerLinear = create_sampler(vk::Filter::eLinear, vk::Filter::eLinear);
        drawImageSamplerNearest = create_sampler(vk::Filter::eNearest, vk::Filter::eNearest);

        primaryDeletionQueue.push_function([this]() {
            device.destroySampler(drawImageSamplerNearest, nullptr);
            device.destroySampler(drawImageSamplerLinear, nullptr);

            device.destroyImageView(drawImage.imageView, nullptr);
            vmaDestroyImage(allocator, drawImage.image, drawImage.allocation);
        });
    }

    void Device::init_depth_images() {
        VkExtent3D depthImageExtent = { width, height, 1 };

        depthImage.imageFormat = vk::Format::eD32Sfloat;
        depthImage.imageExtent = depthImageExtent;

        VkImageUsageFlags depthImageUsages{};
        depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkImageCreateInfo dimg_info  = vkinit::image_create_info(static_cast<VkFormat>(depthImage.imageFormat), depthImageUsages, depthImageExtent);

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vk_check(
            vmaCreateImage(allocator, &dimg_info, &dimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr),
            "Failed to create depth image"
            );

        VkComponentMapping components {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        VkImageViewCreateInfo imageViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageViewCI.image = depthImage.image;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.format = VK_FORMAT_D32_SFLOAT;
        imageViewCI.components = components;
        imageViewCI.subresourceRange = subresourceRange;

        vk_check(
            vkCreateImageView(device, &imageViewCI, nullptr, reinterpret_cast<VkImageView*>(&depthImage.imageView)),
            "Failed to create image view"
        );

        primaryDeletionQueue.push_function([this]() {
            device.destroyImageView(depthImage.imageView, nullptr);
            vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
        });
    }
}
