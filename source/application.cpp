
#include "application.h"

Application::Application() {
    run();
    }

Application::~Application() {

}

void Application::run() {
    VShader shader = device.create_shader("../shaders/test.comp.spv");
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

    GraphicsContext graphicsContext(currentFrame.commandBuffer);
    graphicsContext.begin();
    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    float redValue = std::abs(std::tan(device.frameNumber / 7200.f));
    float greenValue = std::abs(std::acos(std::sin(device.frameNumber / 1000.f)));
    float blueValue = std::abs(std::sin(device.frameNumber / 4720.f));
    clearValue = { { redValue, greenValue, blueValue, 1.0f } };

    VkImageSubresourceRange clearRange =
            vkinit::subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);
    vkCmdClearColorImage(currentFrame.commandBuffer, currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    graphicsContext.image_barrier(currentSwapchainImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    graphicsContext.end();
    device.submit_graphics_work(graphicsContext);
    device.present();
}

void Application::init_descriptors() {

}
