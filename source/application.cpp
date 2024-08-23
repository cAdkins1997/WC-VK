
#include "application.h"

Application::Application() {
    init();
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
    GraphicsContext graphicsContext;
    VkCommandBufferAllocateInfo commandBufferAI = vkinit::command_buffer_AI(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    vkAllocateCommandBuffers(device.device, &commandBufferAI, &commandBuffer);
    graphicsContext.begin();
}

void Application::init() {
    uint32_t graphicsIndex = device.graphicsQueueIndex;
    VkCommandPoolCreateInfo commandPoolCI = vkinit::command_pool_CI(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsIndex);
    vkCreateCommandPool(device.device, &commandPoolCI, nullptr, &commandPool);
}

void Application::init_descriptors() {

}
