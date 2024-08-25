
#include "application.h"

Application::Application() {
    run();
}

Application::~Application() {

}

void Application::run() {
    VBuffer buffer = device.create_buffer(64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    VImage image = device.create_image({ 1920, 1080, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT, true);
    VShader shader = device.create_shader("../shaders/test.comp.spv");
    while (!glfwWindowShouldClose(device.window)) {
        glfwPollEvents();
    }
}

void Application::draw() {
    FrameData& currentFrame = device.get_current_frame();
    uint32_t swapchainImageIndex = device.get_swapchain_image_index();
    VkImage& currentSwapchainImage = device.swapchainImages[swapchainImageIndex];

    GraphicsContext graphicsContext(currentFrame.commandBuffer);
    graphicsContext.begin();
    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    float flash = std::abs(std::sin(device.frameNumber / 120.f));
    clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

    VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(currentFrame.commandBuffer, currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    graphicsContext.end();
    device.submit_graphics_work(graphicsContext);
    device.present();
}

void Application::init_descriptors() {

}
