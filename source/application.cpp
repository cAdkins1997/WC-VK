
#include "application.h"

#include <chrono>

Application::Application() {
    init_immediate_commands();
    run();
}

Application::~Application() {

}

void Application::run() {
    //VShader shader = device.create_shader("../shaders/test.comp.spv");
    //TextureHandle texture = device.create_image({1920, 1080, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);
    while (!glfwWindowShouldClose(device.window)) {
        draw();
        glfwPollEvents();
    }
}

void Application::draw() {
    device.wait_on_work();

    FrameData& currentFrame = device.get_current_frame();
    uint32_t swapchainImageIndex = device.get_swapchain_image_index();
    VkImage& currentSwapchainImage = device.swapchainImages[swapchainImageIndex];
    VkImage& drawImage = device.drawImage.image;

    GraphicsContext graphicsContext(currentFrame.commandBuffer);
    graphicsContext.begin();

    graphicsContext.image_barrier(drawImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    float redValue = std::abs(std::sin(device.frameNumber / 1000000.0f));
    float blueValue = std::abs(std::tan(device.frameNumber / 10000.0f));
    clearValue = { { redValue, 0.1f, blueValue, 1.0f } };

    VkImageSubresourceRange clearRange =
            vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);
    vkCmdClearColorImage(currentFrame.commandBuffer, drawImage, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    graphicsContext.image_barrier(drawImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    graphicsContext.copy_image(drawImage, currentSwapchainImage, {1280, 720}, {1280, 720});
    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    graphicsContext.end();
    device.submit_graphics_work(graphicsContext);
    device.present();
}

void Application::immediate_reset() const {
    vkWaitForFences(device.device, 1, &immediateFence, true, 1000000000);
    vkResetFences(device.device, 1, &immediateFence);
}

void Application::init_immediate_commands() {
    VkCommandPoolCreateInfo commandPoolCI = vkinit::command_pool_CI(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, device.graphicsQueueIndex);
    vkCreateCommandPool(device.device, &commandPoolCI, nullptr, &immediateCommandPool);

    VkCommandBufferAllocateInfo commandBufferAI = vkinit::command_buffer_AI(immediateCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    vkAllocateCommandBuffers(device.device, &commandBufferAI, &immediateCommandBuffer);

    VkFenceCreateInfo fenceCI = vkinit::fence_CI(VK_FENCE_CREATE_SIGNALED_BIT);
    vkCreateFence(device.device, &fenceCI, nullptr, &immediateFence);
}
