
#include "device.hpp"

namespace wcvk {

    void Device::submit_graphics_work(const GraphicsContext &graphicsContext) {
        vk::CommandBuffer cmd = graphicsContext.commandBuffer ;
        vk::CommandBufferSubmitInfo commandBufferSI(cmd);

        FrameData& currentFrame = get_current_frame();
        vk::SemaphoreSubmitInfo waitInfo(currentFrame.swapchainSemaphore);
        waitInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        vk::SemaphoreSubmitInfo signalInfo(currentFrame.renderSemaphore);
        signalInfo.stageMask = vk::PipelineStageFlagBits2::eAllGraphics;
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
        device.resetFences(1, &get_current_frame().renderFence);
    }

    void Device::present() {
        FrameData& currentFrame = get_current_frame();
        vk::PresentInfoKHR presentInfo(1, &currentFrame.renderSemaphore, 1, &swapchain, &swapchainImageIndex);
        graphicsQueue.presentKHR(&presentInfo);
        frameNumber++;
    }

    Device::Device() {
        glfwInit();
        if (glfwVulkanSupported()) {
            uint32_t count;
            const char** extensions = glfwGetRequiredInstanceExtensions(&count);

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(1920, 1080, "Window Title", nullptr, nullptr);

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

            physical_device_selector_ret->enable_extension_if_present("VK_EXT_descriptor_buffer");
            auto extensionsPossible = physical_device_selector_ret->get_extensions();
            for (const auto& extension : extensionsPossible) {
                std::cout << extension;
            }

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

        vmaDestroyImage(allocator, drawImage.image, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);

        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);

        glfwDestroyWindow(window);

        vkDestroyInstance(instance, nullptr);

        glfwTerminate();
    }

    void Device::init_commands() {
        vk::CommandPoolCreateInfo commandPoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueIndex);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frames[i].commandPool = device.createCommandPool(commandPoolCI, nullptr);
            vk::CommandBufferAllocateInfo commandBufferAI(frames[i].commandPool, vk::CommandBufferLevel::ePrimary, 1);
            device.allocateCommandBuffers(&commandBufferAI, &frames[i].commandBuffer);
        }
    }

    void Device::init_sync_objects() {
        vk::FenceCreateInfo fenceCI(vk::FenceCreateFlagBits::eSignaled);
        vk::SemaphoreCreateInfo semaphoreCI;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            device.createFence(&fenceCI, nullptr, &frames[i].renderFence);
            device.createSemaphore(&semaphoreCI, nullptr, &frames[i].swapchainSemaphore);
            device.createSemaphore(&semaphoreCI, nullptr, &frames[i].renderSemaphore);
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
        VkPhysicalDeviceProperties2 deviceProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR };
        VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT };
        deviceProperties.pNext = &descriptorBufferProperties;
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
    }

    void Device::init_draw_images() {
        VkExtent3D drawImageExtent = { width, height, 1 };

        drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        drawImage.imageExtent = drawImageExtent;

        VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkImageCreateInfo rimg_info = vkinit::image_create_info(drawImage.imageFormat, drawImageUsages, drawImageExtent);

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
        VkImageViewCreateInfo viewInfo = vkinit::image_view_CI(drawImage.image, VK_IMAGE_VIEW_TYPE_2D, drawImage.imageFormat, components, subresourceRange);

        vkCreateImageView(device, &viewInfo, nullptr, &drawImage.imageView);
    }
}
