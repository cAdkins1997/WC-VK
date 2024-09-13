
#include "application.h"
#include <chrono>

namespace wcvk {
Application::Application() {
    run();
}

Application::~Application() {

}

void Application::run() {
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

    graphicsContext.image_barrier(drawImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

    float redValue = std::abs(std::sin(device.frameNumber / 100000.0f));
    float blueValue = std::abs(std::tan(device.frameNumber / 10000.0f));
    vk::ClearColorValue clearValue{ redValue, 0.01f, blueValue, 1.0f };

    vk::ImageSubresourceRange clearRange(vk::ImageAspectFlagBits::eColor, 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers);
    currentFrame.commandBuffer.clearColorImage(drawImage, vk::ImageLayout::eGeneral, &clearValue, 1, &clearRange);

    graphicsContext.image_barrier(drawImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);

    graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    graphicsContext.copy_image(drawImage, currentSwapchainImage, {1920, 1080}, {1920, 1080});
    graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

    graphicsContext.end();
    device.submit_graphics_work(graphicsContext);
    device.present();
}

}